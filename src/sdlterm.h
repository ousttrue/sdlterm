#pragma once
#include <SDL.h>
#include <SDL_fox.h>

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
};

struct TERM_State {
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
  class VTermApp *vterm_ = nullptr;
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

  ~TERM_State();
  bool Initialize(TERM_Config *cfg, const char *title);
  bool HandleEvents();
  void Update();
private:
  void RenderScreen();
  void RenderCursor();
  void RenderCell(int x, int y);
};
