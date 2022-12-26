#pragma once
#include "term_config.h"
#include <SDL.h>
#include <functional>
#include <vector>

class SDLTermWindow {
  SDL_Cursor *pointer_;
  SDL_Surface *icon_;
  const Uint8 *keys_;
  // SDL_Rect mouse_rect_ = {0};
  // bool mouse_down_ = false;

  SDL_Window *window_;
  int width_ = 0;
  int height_ = 0;

public:
  std::vector<char> keyInputBuffer_;
  SDLTermWindow();
  ~SDLTermWindow();
  SDL_Window *Initialize(TERM_Config *cfg, const char *title);
  int Width() const { return width_; }
  int Height() const { return height_; }
  bool HandleEvents();

private:
  void HandleKeyEvent(SDL_Event *event);
  void HandleWindowEvent(SDL_Event *event);
};
