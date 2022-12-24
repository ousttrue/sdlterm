#include "term_config.h"
#include <SDL.h>
#include <stdio.h>
#ifdef _MSC_VER
#else
#include <unistd.h>
#endif

#define COPYRIGHT                                                              \
  "Copyright (c) 2020 Niklas Benfer <https://github.com/palomena>"

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

static const char options[] = "hvlx:y:f:b:s:r:w:e:";
static const char version[] = {PROGNAME "\n" COPYRIGHT};

static void TERM_ListRenderBackends(void) {
  int num = SDL_GetNumRenderDrivers();
  printf("Available SDL renderer backends:\n");
  for (int i = 0; i < num; i++) {
    SDL_RendererInfo info;
    SDL_GetRenderDriverInfo(i, &info);
    printf("%d: %s\n", i, info.name);
  }
}

int TERM_Config::ParseArgs(int argc, char **argv) {
  int status = 0;

#ifdef _MSC_VER
#else

  int option;
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
        this->width = strtol(optarg, NULL, 10);
      break;
    case 'y':
      if (optarg != NULL)
        this->height = strtol(optarg, NULL, 10);
      break;
    case 'f':
      if (optarg != NULL)
        this->fontpattern = optarg;
      break;
    case 'b':
      if (optarg != NULL)
        this->boldfontpattern = optarg;
      break;
    case 'r':
      if (optarg != NULL)
        this->renderer = optarg;
      break;
    case 'w':
      if (optarg != NULL) {
        this->windowflags[this->nWindowFlags++] = optarg;
      }
      break;
    case 'e':
      if (optarg != NULL)
        this->exec = optarg;
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
    this->args = &argv[optind];
  }
#endif
  return status;
}
