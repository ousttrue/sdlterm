#include "sdlrenderer.h"
#include "term_config.h"
#include <iostream>

#include <childprocess.h>
#include <sdl_app.h>
#include <vterm_object.h>

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
  int font_width = renderer->font_metrics->max_advance;
  int font_height = renderer->font_metrics->height;
  int rows = window->Height() / font_height;
  int cols = window->Width() / font_width;

  // child
  termtk::ChildProcess child;
  child.Launch(rows, cols, cfg.exec);

  termtk::Terminal vterm(rows, cols, font_width, font_height,
                         &termtk::ChildProcess::Write, &child);

  while (app.NewFrame()) {
    if (child.IsClosed()) {
      break;
    }

    {
      auto input = child.Read();
      if (!input.empty()) {
        vterm.input_write(input.data(), input.size());
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
      vterm.set_rows_cols(rows, cols);
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
          if (auto cell = vterm.get_cell(pos)) {
            renderer->RenderCell(pos, *cell);
          }
        }
      }
    }
    renderer->EndRender(render_screen, cfg.width, cfg.height);
  }

  return 0;
}
