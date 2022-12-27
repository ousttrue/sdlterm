#include "childprocess.h"
#include "sdlrenderer.h"
#include "term_config.h"
#include "vterm.h"
#include "vtermapp.h"
#include <iostream>
#include <sdl_app.h>

int main(int argc, char *argv[]) {
  TERM_Config cfg = {};
  if (cfg.ParseArgs(argc, argv)) {
    return 1;
  }

  termtk::SDLApp app;
  auto window = app.CreateWindow(640, 480, PROGNAME);
  if (!window) {
    return 2;
  }

  auto renderer = SDLRenderer::Create(window->Handle());
  if (!renderer) {
    return 3;
  }
  if (!renderer->LoadFont(cfg.font, cfg.fontsize, cfg.boldfont)) {
    return 4;
  }
  int rows = window->Height() / renderer->font_metrics->height;
  int cols = window->Width() / renderer->font_metrics->max_advance;

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

  while (app.NewFrame()) {
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
    auto input = app.DequeueInput();
    if (!input.empty()) {
      child.Write(input.data(), input.size());
    }

    // window size to rows & cols
    int new_cols = window->Width() / renderer->font_metrics->max_advance;
    int new_rows = window->Height() / renderer->font_metrics->height;
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
