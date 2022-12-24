#pragma once
#include <SDL.h>
#include <functional>
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
  SDL_Cursor *pointer;
  SDL_Surface *icon;
  const Uint8 *keys;
  SDL_Rect mouse_rect;
  bool mouse_clicked;
  pid_t child;
  int childfd;
  TERM_Config cfg;

public:
  std::shared_ptr<class SDLRenderer> renderer_;
  std::function<void(const char *, size_t)> ChildOutputCallback;
  std::function<uint32_t(int row, int col, struct CellState *)> GetCellCallback;
  std::function<void(int rows, int cols)> RowsColsChanged;
  std::function<size_t(char *buf, size_t len, const struct TERM_Rect &rect)>
      GetTextCallback;

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
