#pragma once

#define PROGNAME "sdlterm version 0.1"

struct TERM_Config {
  char **args = nullptr;

#ifdef _MSC_VER
  const char *font = "C:/Windows/Fonts/consola.ttf";
  const char *boldfont = "C:/Windows/Fonts/consolab.ttf";
  // const char* exec = "pwsh.exe";
  const char *exec = "cmd.exe";
#else
  const char *boldfont = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
  const char *font = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf";
  const char *exec = "/bin/bash";
#endif

  const char *windowflags[5] = {0};
  int nWindowFlags = 0;
  int fontsize = 16;
  int width = 800;
  int height = 600;
  int rows = 24;
  int columns = 80;

  int ParseArgs(int argc, char **argv);
};
