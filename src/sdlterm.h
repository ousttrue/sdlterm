#pragma once
#include "childprocess.h"
#include "term_config.h"
#include <SDL.h>
#include <functional>
#include <memory>

class SDLTermWindow {
  SDL_Window *window_;
  SDL_Cursor *pointer_;
  SDL_Surface *icon_;
  const Uint8 *keys_;
  SDL_Rect mouse_rect_;
  bool mouse_down_;
  TERM_Config cfg_;

public:
  ChildProcess child_;
  std::shared_ptr<class SDLRenderer> renderer_;
  std::function<uint32_t(int row, int col, struct CellState *)> GetCellCallback;
  std::function<void(int rows, int cols)> RowsColsChanged;
  std::function<size_t(char *buf, size_t len, const struct TERM_Rect &rect)>
      GetTextCallback;

  SDLTermWindow();
  ~SDLTermWindow();
  bool Initialize(TERM_Config *cfg, const char *title);
  bool HandleEvents();
  void Update();
  void Resize(int width, int height);

private:
  void HandleKeyEvent(SDL_Event *event);
  void HandleWindowEvent(SDL_Event *event);
};
