#pragma once
#include <SDL.h>

struct TERM_Rect {
  int start_row;
  int end_row;
  int start_col;
  int end_col;

  static TERM_Rect FromMouseRect(const SDL_Rect &mouse_rect, int font_height,
                                 int font_width);
};
