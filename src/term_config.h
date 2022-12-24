#pragma once

#define PROGNAME "sdlterm version 0.1"

struct TERM_Config {
  const char *exec;
  char **args;
  const char *fontpattern;
  const char *boldfontpattern;
  const char *renderer;
  const char *windowflags[5];
  int nWindowFlags;
  int fontsize;
  int width;
  int height;
  int rows;
  int columns;

  int ParseArgs(int argc, char **argv);
};
