#pragma once
#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <vterm.h>

template <> struct ::std::hash<VTermPos> {
  std::size_t operator()(const VTermPos &p) const noexcept {
    return p.row << 16 | p.col;
  }
};

namespace termtk {
struct PosEqual {
  constexpr bool operator()(VTermPos lhs, VTermPos rhs) const {
    return lhs.row == rhs.row && lhs.col == rhs.col;
  }
};

using PosSet = std::unordered_set<::VTermPos, std::hash<::VTermPos>, PosEqual>;

class Terminal {
  VTerm *vterm_;
  VTermScreen *screen_;
  VTermPos cursor_pos_;
  mutable VTermScreenCell cell_;
  bool ringing_ = false;

  PosSet damaged_;
  PosSet tmp_;

public:
  Terminal(int _rows, int _cols, int font_width, int font_height,
           VTermOutputCallback out, void *user);
  ~Terminal();
  void input_write(const char *bytes, size_t len);
  void keyboard_unichar(char c, VTermModifier mod);
  void keyboard_key(VTermKey key, VTermModifier mod);
  const PosSet &new_frame(bool *ringing);
  VTermScreenCell *get_cell(VTermPos pos) const;
  VTermScreenCell *get_cursor(VTermPos *pos) const;
  void set_rows_cols(int rows, int cols);

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

  int damage(int start_row, int start_col, int end_row, int end_col);
  int moverect(VTermRect dest, VTermRect src);
  int movecursor(VTermPos pos, VTermPos oldpos, int visible);
  int settermprop(VTermProp prop, VTermValue *val);
  int bell();
  int resize(int rows, int cols);
  int sb_pushline(int cols, const VTermScreenCell *cells);
  int sb_popline(int cols, VTermScreenCell *cells);
};
} // namespace termtk