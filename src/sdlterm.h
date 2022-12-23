#pragma once
#include <SDL.h>
#include <SDL_fox.h>
#include <vterm.h>

typedef struct {
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
} TERM_Config;

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Cursor *pointer;
  SDL_Surface *icon;
  const Uint8 *keys;
  struct {
    const FOX_FontMetrics *metrics;
    FOX_Font *regular;
    FOX_Font *bold;
  } font;
  VTerm *vterm;
  VTermScreen *screen;
  VTermState *termstate;
  struct {
    SDL_Point position;
    bool visible;
    bool active;
    Uint32 ticks;
  } cursor;
  struct {
    SDL_Rect rect;
    bool clicked;
  } mouse;
  struct {
    Uint32 ticks;
    bool active;
  } bell;
  Uint32 ticks;
  bool dirty;
  pid_t child;
  int childfd;
  TERM_Config cfg;
} TERM_State;

int TERM_InitializeTerminal(TERM_State *state, TERM_Config *cfg, const char *title);
int TERM_HandleEvents(TERM_State *state);
void TERM_Update(TERM_State *state);
void TERM_DeinitializeTerminal(TERM_State *state);
