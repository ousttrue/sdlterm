#pragma once
#include "SDL_stdinc.h"
#include "TERM_Rect.h"
#include <functional>
#include <vterm.h>

class VTermApp {
  VTerm *vterm = nullptr;
  VTermScreen *screen = nullptr;
  VTermState *termstate = nullptr;

public:
  std::function<void()> BellCallback;
  std::function<void(int row, int col, bool visible)> MoveCursorCallback;

  ~VTermApp();
  void Initialize(int row, int col);
  void Write(const char *bytes, size_t len);
  size_t GetText(char *buffer, size_t len, const TERM_Rect &rect);
  void Resize(int rows, int cols);
  uint32_t Cell(int row, int col, struct CellState *pCell);
};
