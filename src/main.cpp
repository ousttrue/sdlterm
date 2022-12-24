#include "sdlrenderer.h"
#include "sdlterm.h"
#include "vtermapp.h"
#include <functional>
#include <unistd.h>

#define PROGNAME "sdlterm version 0.1"
#define COPYRIGHT                                                              \
  "Copyright (c) 2020 Niklas Benfer <https://github.com/palomena>"

auto FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
auto BOLD_FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf";

static const char options[] = "hvlx:y:f:b:s:r:w:e:";

static const char help[] = {
    PROGNAME
    "\n" COPYRIGHT "\n"
    "sdlterm usage:\n"
    "\tsdlterm [option...]* [child options...]*\n"
    "Options:\n"
    "  -h\tDisplay help text\n"
    "  -v\tDisplay program version\n"
    "  -x\tSet window width in pixels\n"
    "  -y\tSet window height in pixels\n"
    "  -f\tSet regular font via path (fontconfig pattern not yet supported)\n"
    "  -b\tSet bold font via path (fontconfig pattern not yet supported)\n"
    "  -s\tSet fontsize\n"
    "  -r\tSet SDL renderer backend\n"
    "  -l\tList available SDL renderer backends\n"
    "  -w\tSet SDL window flags\n"
    "  -e\tSet child process executable path\n"};

static const char version[] = {PROGNAME "\n" COPYRIGHT};

static void TERM_ListRenderBackends(void) {
  int num = SDL_GetNumRenderDrivers();
  puts("Available SDL renderer backends:");
  for (int i = 0; i < num; i++) {
    SDL_RendererInfo info;
    SDL_GetRenderDriverInfo(i, &info);
    printf("%d: %s\n", i, info.name);
  }
}

static int ParseArgs(TERM_Config *cfg, int argc, char **argv) {
  int option;
  int status = 0;

  while ((option = getopt(argc, argv, options)) != -1) {
    switch (option) {
    case 'h':
      puts(help);
      status = 1;
      break;
    case 'v':
      puts(version);
      status = 1;
      break;
    case 'x':
      if (optarg != NULL)
        cfg->width = strtol(optarg, NULL, 10);
      break;
    case 'y':
      if (optarg != NULL)
        cfg->height = strtol(optarg, NULL, 10);
      break;
    case 'f':
      if (optarg != NULL)
        cfg->fontpattern = optarg;
      break;
    case 'b':
      if (optarg != NULL)
        cfg->boldfontpattern = optarg;
      break;
    case 'r':
      if (optarg != NULL)
        cfg->renderer = optarg;
      break;
    case 'w':
      if (optarg != NULL) {
        cfg->windowflags[cfg->nWindowFlags++] = optarg;
      }
      break;
    case 'e':
      if (optarg != NULL)
        cfg->exec = optarg;
      break;
    case 'l':
      TERM_ListRenderBackends();
      status = 1;
      break;
    default:
      status = 1;
      break;
    }
  }
  if (optind < argc) {
    cfg->args = &argv[optind];
  }

  return status;
}

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

  if (ParseArgs(&cfg, argc, argv)) {
    return 1;
  }

  SDLApp state;
  VTermApp vterm;

  vterm.Initialize(cfg.rows, cfg.columns);
  state.RowsColsChanged = std::bind(
      &VTermApp::Resize, &vterm, std::placeholders::_1, std::placeholders::_2);

  if (!state.Initialize(&cfg, PROGNAME)) {
    return 2;
  }

  vterm.BellCallback = std::bind(&SDLRenderer::SetBell, state.renderer_);
  vterm.MoveCursorCallback = std::bind(
      &SDLRenderer::MoveCursor, state.renderer_, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3);

  state.ChildOutputCallback = std::bind(
      &VTermApp::Write, &vterm, std::placeholders::_1, std::placeholders::_2);

  state.GetCellCallback =
      std::bind(&VTermApp::Cell, &vterm, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3);

  state.GetTextCallback =
      std::bind(&VTermApp::GetText, &vterm, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3);

  while (state.HandleEvents()) {
    state.Update();
  }

  return 0;
}
