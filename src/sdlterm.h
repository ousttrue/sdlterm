#pragma once
#include <SDL.h>
#include <memory>

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
  std::shared_ptr<class SDLRenderer> renderer_;
  SDL_Cursor *pointer;
  SDL_Surface *icon;
  const Uint8 *keys;
  class VTermApp *vterm_ = nullptr;
  SDL_Rect mouse_rect;
  bool mouse_clicked;
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
};
