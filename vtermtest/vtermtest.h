#pragma once
#include <functional>
#include <memory>
#include <stdexcept>
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

class Terminal {
  VTerm *vterm_;
  VTermScreen *screen_;
  VTermPos cursor_pos_;
  mutable VTermScreenCell cell_;
  bool ringing_ = false;
  bool isInvalidated_ = true;
  Matrix<unsigned char> matrix_;

public:
  Terminal(int _rows, int _cols, int font_width, int font_height,
           VTermOutputCallback out, void *user);
  ~Terminal();
  void input_write(const char *bytes, size_t len);
  void keyboard_unichar(char c, VTermModifier mod);
  void keyboard_key(VTermKey key, VTermModifier mod);

  Matrix<unsigned char> *new_frame(bool *ringing) {
    auto current = isInvalidated_;
    isInvalidated_ = false;
    *ringing = ringing_;
    ringing_ = false;
    return current ? &matrix_ : nullptr;
  }
  VTermScreenCell *get_cell(VTermPos pos) const;
  VTermScreenCell *get_cursor(VTermPos *pos) const;

private:
  static int damage(VTermRect rect, void *user);
  static int moverect(VTermRect dest, VTermRect src, void *user);
  static int movecursor(VTermPos pos, VTermPos oldpos, int visible, void *user);
  static int settermprop(VTermProp prop, VTermValue *val, void *user);
  static int bell(void *user);
  static int resize(int rows, int cols, void *user);
  static int sb_pushline(int cols, const VTermScreenCell *cells, void *user);
  static int sb_popline(int cols, VTermScreenCell *cells, void *user);
  const VTermScreenCallbacks screen_callbacks = {
      damage, moverect, movecursor,  settermprop,
      bell,   resize,   sb_pushline, sb_popline};

  void invalidateTexture() { isInvalidated_ = true; }
  int damage(int start_row, int start_col, int end_row, int end_col);
  int moverect(VTermRect dest, VTermRect src);
  int movecursor(VTermPos pos, VTermPos oldpos, int visible);
  int settermprop(VTermProp prop, VTermValue *val);
  int bell();
  int resize(int rows, int cols);
  int sb_pushline(int cols, const VTermScreenCell *cells);
  int sb_popline(int cols, VTermScreenCell *cells);
};
