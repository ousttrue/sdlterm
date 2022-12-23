#pragma once
#include <functional>
#include <vterm.h>

class VTermApp {
  VTerm *vterm = nullptr;
  VTermScreen *screen = nullptr;
  VTermState *termstate = nullptr;
  VTermScreenCell cell = {};

public:
  ~VTermApp();
  void Initialize(int row, int col);
  void Write(const char *bytes, size_t len);
  size_t GetText(char *buffer, size_t len, const VTermRect &rect);
  VTermScreenCell *GetCell(const VTermPos &pos);
  void UpdateCell(VTermScreenCell *cell);
  void Resize(int rows, int cols);

  std::function<void()> BellCallback;
  std::function<void(int row, int col, bool visible)> MoveCursorCallback;
};
