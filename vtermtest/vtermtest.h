#pragma once
#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <vterm.h>

struct PosEqual {
  constexpr bool operator()(VTermPos lhs, VTermPos rhs) const {
    return lhs.row == rhs.row && lhs.col == rhs.col;
  }
};
namespace detail // 便利ライブラリを置く名前空間（名前は任意）
{
// 複数のハッシュ値を組み合わせて新しいハッシュ値を作る関数
// 実装出典: Boost.ContainerHash
// https://github.com/boostorg/container_hash/blob/develop/include/boost/container_hash/hash.hpp
inline void HashCombineImpl(std::size_t &h, std::size_t k) noexcept {
  static_assert(sizeof(std::size_t) == 8); // 要 64-bit 環境
  constexpr std::uint64_t m = 0xc6a4a7935bd1e995;
  constexpr int r = 47;
  k *= m;
  k ^= k >> r;
  k *= m;
  h ^= k;
  h *= m;
  // Completely arbitrary number, to prevent 0's
  // from hashing to 0.
  h += 0xe6546b64;
}

// 複数のハッシュ値を組み合わせて新しいハッシュ値を作る関数
template <class Type>
inline void HashCombine(std::size_t &h, const Type &value) noexcept {
  HashCombineImpl(h, std::hash<Type>{}(value));
}
} // namespace detail
template <>
struct std::hash<VTermPos> // std::hash の特殊化
{
  std::size_t operator()(const VTermPos &p) const noexcept {
    std::size_t seed = 0;
    detail::HashCombine(seed, p.row); // ハッシュ値を更新
    detail::HashCombine(seed, p.col); // ハッシュ値を更新
    return seed;
  }
};

using PosSet = std::unordered_set<VTermPos, std::hash<VTermPos>, PosEqual>;
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
