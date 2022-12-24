#include "sdlrenderer.h"
#include "sdlterm.h"
#include "term_config.h"
#include "vtermapp.h"
#include <functional>
#include <stdexcept>

auto FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
auto BOLD_FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf";

struct SDLApp {
  SDLApp() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
      throw std::runtime_error("SDL_Init");
    }
  }
  ~SDLApp() { SDL_Quit(); }
};

int main(int argc, char *argv[]) {

  TERM_Config cfg = {.exec = "/bin/bash",
                     .args = NULL,
                     .fontpattern = FONT,
                     .boldfontpattern = BOLD_FONT,
                     .renderer = NULL,
                     .windowflags = {NULL},
                     .nWindowFlags = 0,
                     .fontsize = 16,
                     .width = 800,
                     .height = 600,
                     .rows = 0,
                     .columns = 0};

  if (cfg.ParseArgs(argc, argv)) {
    return 1;
  }

  SDLApp app;
  VTermApp vterm;
  vterm.Initialize(cfg.rows, cfg.columns);

  SDLTermWindow window;
  window.RowsColsChanged = std::bind(
      &VTermApp::Resize, &vterm, std::placeholders::_1, std::placeholders::_2);
  if (!window.Initialize(&cfg, PROGNAME)) {
    return 2;
  }

  // vterm -> window
  vterm.BellCallback = std::bind(&SDLRenderer::SetBell, window.renderer_.get());
  vterm.MoveCursorCallback = std::bind(
      &SDLRenderer::MoveCursor, window.renderer_.get(), std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3);

  // childprocess output
  window.child_.OutputCallback = std::bind(
      &VTermApp::Write, &vterm, std::placeholders::_1, std::placeholders::_2);
  window.child_.OutputCallback = [&window](const char *, size_t) {
    window.renderer_->SetDirty();
  };

  // window -> vterm
  window.GetCellCallback =
      std::bind(&VTermApp::Cell, &vterm, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3);

  window.GetTextCallback =
      std::bind(&VTermApp::GetText, &vterm, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3);

  while (window.HandleEvents()) {
    window.Update();
  }

  return 0;
}
