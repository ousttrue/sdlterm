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

class SDLApp {
  SDL_Window *window;
  SDL_Renderer *renderer_;
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

public:
  SDLApp();
  ~SDLApp();
  bool Initialize(TERM_Config *cfg, const char *title);
  bool HandleEvents();
  void Update();
  void Resize(int width, int height);

private:
  void HandleKeyEvent(SDL_Event *event);
  void HandleWindowEvent(SDL_Event *event);
  void HandleChildEvents();

  void RenderScreen();
  void RenderCursor();
  void RenderCell(int x, int y);
};
