#include "childprocess.h"
#include "sdlrenderer.h"
#include "sdlterm.h"
#include "term_config.h"
#include "vterm.h"
#include "vtermapp.h"
#include <SDL_fox.h>
#include <functional>
#include <stdexcept>
#include <iostream>

struct SDLApp {
  SDLApp() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
      throw std::runtime_error("SDL_Init");
    }
    if (FOX_Init() != FOX_INITIALIZED) {
      throw std::runtime_error("FOX_Init");
    }
  }
  ~SDLApp() {
    FOX_Exit();

    SDL_Quit();
  }
};

int main(int argc, char *argv[]) {
  TERM_Config cfg = {};
  if (cfg.ParseArgs(argc, argv)) {
    return 1;
  }

  SDLApp app;

  SDLTermWindow window;
  auto window_handle = window.Initialize(&cfg, PROGNAME);
  if (!window_handle) {
    return 2;
  }

  auto renderer = SDLRenderer::Create(window_handle);
  if (!renderer) {
    return 3;
  }
  if (!renderer->LoadFont(cfg.font, cfg.fontsize, cfg.boldfont)) {
    return 4;
  }
  int rows = window.Height() / renderer->font_metrics->height;
  int cols = window.Width() / renderer->font_metrics->max_advance;

  VTermApp vterm;
  vterm.Initialize(rows, cols);

  // vterm -> window
  vterm.BellCallback = std::bind(&SDLRenderer::SetBell, renderer.get());
  vterm.MoveCursorCallback =
      std::bind(&SDLRenderer::MoveCursor, renderer.get(), std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3);

  // child
  ChildProcess child;
  if (!child.Launch(cfg.exec, cfg.args)) {
    return 5;
  }

  while (window.HandleEvents()) {
    if (child.Closed()) {
      break;
    }

    // child output to vterm
    {
      size_t read_size;
      auto p = child.Read(&read_size);
      if (read_size) {
        vterm.Write(p, read_size);
        renderer->SetDirty();
      }
    }

    // window input to child
    if (!window.keyInputBuffer_.empty()) {
      child.Write(window.keyInputBuffer_.data(), window.keyInputBuffer_.size());
      window.keyInputBuffer_.clear();
      // renderer->SetDirty();
    }

    // window size to rows & cols
    int new_cols = window.Width() / renderer->font_metrics->max_advance;
    int new_rows = window.Height() / renderer->font_metrics->height;
    if (new_rows != rows || new_cols != cols) {
      rows = new_rows;
      cols = new_cols;
      std::cout << "rows x cols: " << rows << " x " << cols << std::endl;
      vterm.Resize(rows, cols);
      child.NotifyTermSize(rows, cols);
      renderer->SetDirty();
    }

    // render vterm
    auto render_screen = renderer->BeginRender();
    if (render_screen) {
      for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
          VTermPos pos = {
              .row = y,
              .col = x,
          };
          if (auto cell = vterm.Cell(pos)) {
            renderer->RenderCell(pos, *cell);
          }
        }
      }
    }
    renderer->EndRender(render_screen, cfg.width, cfg.height);
  }

  return 0;
}
