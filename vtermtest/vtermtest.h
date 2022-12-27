#pragma once
#include "SDL_pixels.h"
#include "SDL_surface.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>
#include <vterm.h>

template <typename T> class Matrix {
  T *buf;
  int rows, cols;

public:
  Matrix(int _rows, int _cols) : rows(_rows), cols(_cols) {
    buf = new T[cols * rows];
  }
  ~Matrix() { delete buf; }
  void fill(const T &by) {
    for (int row = 0; row < rows; row++) {
      for (int col = 0; col < cols; col++) {
        buf[cols * row + col] = by;
      }
    }
  }
  T &operator()(int row, int col) {
    if (row < 0 || col < 0 || row >= rows || col >= cols)
      throw std::runtime_error("invalid position");
    // else
    return buf[cols * row + col];
  }
  int getRows() const { return rows; }
  int getCols() const { return cols; }
};

using CellSurface =
    std::function<SDL_Surface *(const VTermScreenCell &cell, SDL_Color color)>;

class Terminal {
  VTerm *vterm_;
  VTermScreen *screen_;
  SDL_Surface *surface_ = NULL;
  SDL_Texture *texture_ = NULL;
  Matrix<unsigned char> matrix_;
  int fd_;
  bool ringing_ = false;

  VTermPos cursor_pos_;

public:
  Terminal(int _fd, int _rows, int _cols, int font_width, int font_height);
  ~Terminal();
  void invalidateTexture();
  void keyboard_unichar(char c, VTermModifier mod);
  void keyboard_key(VTermKey key, VTermModifier mod);
  void input_write(const char *bytes, size_t len);
  int damage(int start_row, int start_col, int end_row, int end_col);
  int moverect(VTermRect dest, VTermRect src);
  int movecursor(VTermPos pos, VTermPos oldpos, int visible);
  int settermprop(VTermProp prop, VTermValue *val);
  int bell();
  int resize(int rows, int cols);
  int sb_pushline(int cols, const VTermScreenCell *cells);
  int sb_popline(int cols, VTermScreenCell *cells);
  void render(SDL_Renderer *renderer, const SDL_Rect &window_rect,
              const CellSurface &cellSurface, int font_width, int font_height);
  void render_cell(VTermPos pos, const CellSurface &cellSurface, int font_width,
                   int font_height);
  void processEvent(const SDL_Event &ev);
  void processInput();
};

std::pair<int, int>
createSubprocessWithPty(int rows, int cols, const char *prog,
                        const std::vector<std::string> &args = {},
                        const char *TERM = "xterm-256color");

std::pair<pid_t, int> waitpid(pid_t pid, int options);