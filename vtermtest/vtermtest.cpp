#include "vtermtest.h"
#include "SDL_video.h"
#include "vterm.h"
#include <iostream>
#include <string.h>

int Terminal::damage(VTermRect rect, void *user) {
  return ((Terminal *)user)
      ->damage(rect.start_row, rect.start_col, rect.end_row, rect.end_col);
}

int Terminal::moverect(VTermRect dest, VTermRect src, void *user) {
  return ((Terminal *)user)->moverect(dest, src);
}

int Terminal::movecursor(VTermPos pos, VTermPos oldpos, int visible,
                         void *user) {
  return ((Terminal *)user)->movecursor(pos, oldpos, visible);
}

int Terminal::settermprop(VTermProp prop, VTermValue *val, void *user) {
  return ((Terminal *)user)->settermprop(prop, val);
}

int Terminal::bell(void *user) { return ((Terminal *)user)->bell(); }

int Terminal::resize(int rows, int cols, void *user) {
  return ((Terminal *)user)->resize(rows, cols);
}

int Terminal::sb_pushline(int cols, const VTermScreenCell *cells, void *user) {
  return ((Terminal *)user)->sb_pushline(cols, cells);
}

int Terminal::sb_popline(int cols, VTermScreenCell *cells, void *user) {
  return ((Terminal *)user)->sb_popline(cols, cells);
}

Terminal::Terminal(int _rows, int _cols, int font_width, int font_height,
                   VTermOutputCallback out, void *user)
    : matrix_(_rows, _cols) {
  vterm_ = vterm_new(_rows, _cols);
  vterm_set_utf8(vterm_, 1);
  vterm_output_set_callback(vterm_, out, user);

  screen_ = vterm_obtain_screen(vterm_);
  vterm_screen_set_callbacks(screen_, &screen_callbacks, this);
  vterm_screen_reset(screen_, 1);

  matrix_.fill(0);
  surface_ = SDL_CreateRGBSurfaceWithFormat(
      0, font_width * _cols, font_height * _rows, 32, SDL_PIXELFORMAT_RGBA32);

  SDL_CreateRGBSurface(0, font_width, font_height, 32, 0, 0, 0, 0);
  // SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
}

Terminal::~Terminal() {
  vterm_free(vterm_);
  invalidateTexture();
  SDL_FreeSurface(surface_);
}

void Terminal::invalidateTexture() {
  if (texture_) {
    SDL_DestroyTexture(texture_);
    texture_ = NULL;
  }
}

void Terminal::keyboard_unichar(char c, VTermModifier mod) {
  vterm_keyboard_unichar(vterm_, c, mod);
}

void Terminal::keyboard_key(VTermKey key, VTermModifier mod) {
  vterm_keyboard_key(vterm_, key, mod);
}

void Terminal::input_write(const char *bytes, size_t len) {
  vterm_input_write(vterm_, bytes, len);
}

int Terminal::damage(int start_row, int start_col, int end_row, int end_col) {
  invalidateTexture();
  for (int row = start_row; row < end_row; row++) {
    for (int col = start_col; col < end_col; col++) {
      matrix_(row, col) = 1;
    }
  }
  return 0;
}

int Terminal::moverect(VTermRect dest, VTermRect src) {
  std::cout << "moverect" << std::endl;
  return 0;
}

int Terminal::movecursor(VTermPos pos, VTermPos oldpos, int visible) {
  cursor_pos_ = pos;
  return 0;
}

int Terminal::settermprop(VTermProp prop, VTermValue *val) {
  switch (prop) {
  case VTERM_PROP_CURSORVISIBLE:
    // bool
    std::cout << "VTERM_PROP_CURSORVISIBLE: " << val->boolean << std::endl;
    break;
  case VTERM_PROP_CURSORBLINK:
    // bool
    std::cout << "VTERM_PROP_CURSORBLINK: " << val->boolean << std::endl;
    break;
  case VTERM_PROP_ALTSCREEN:
    // bool
    std::cout << "VTERM_PROP_ALTSCREEN: " << val->boolean << std::endl;
    break;
  case VTERM_PROP_TITLE:
    // string
    std::cout << "VTERM_PROP_TITLE: " << std::string_view(val->string.str, val->string.len) << std::endl;
    break;
  case VTERM_PROP_ICONNAME:
    // string
    std::cout << "VTERM_PROP_ICONNAME: " << std::string_view(val->string.str, val->string.len) << std::endl;
    break;
  case VTERM_PROP_REVERSE:
    // bool
    std::cout << "VTERM_PROP_REVERSE: " << val->boolean << std::endl;
    break;
  case VTERM_PROP_CURSORSHAPE:
    // number
    std::cout << "VTERM_PROP_CURSORSHAPE: " << val->number << std::endl;
    break;
  case VTERM_PROP_MOUSE:
    // number
    std::cout << "VTERM_PROP_MOUSE: " << val->number << std::endl;
    break;
  default:
    std::cout << "unknown prop: " << prop << std::endl;
  }
  return 0;
}

int Terminal::bell() {
  ringing_ = true;
  return 0;
}

int Terminal::resize(int rows, int cols) {
  std::cout << "resize" << std::endl;
  return 0;
}

int Terminal::sb_pushline(int cols, const VTermScreenCell *cells) {
  std::cout << "sb_pushline" << std::endl;
  return 0;
}

int Terminal::sb_popline(int cols, VTermScreenCell *cells) {
  std::cout << "sb_popline" << std::endl;
  return 0;
}

void Terminal::render(SDL_Renderer *renderer, const SDL_Rect &window_rect,
                      const CellSurface &cellSurface, int font_width,
                      int font_height) {
  if (!texture_) {
    for (int row = 0; row < matrix_.getRows(); row++) {
      for (int col = 0; col < matrix_.getCols(); col++) {
        if (matrix_(row, col)) {
          VTermPos pos = {row, col};
          render_cell(pos, cellSurface, font_width, font_height);
          matrix_(row, col) = 0;
        }
      }
    }
    texture_ = SDL_CreateTextureFromSurface(renderer, surface_);
    SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
  }
  SDL_RenderCopy(renderer, texture_, NULL, &window_rect);
  // draw cursor
  VTermScreenCell cell;
  vterm_screen_get_cell(screen_, cursor_pos_, &cell);

  SDL_Rect rect = {cursor_pos_.col * font_width, cursor_pos_.row * font_height,
                   font_width, font_height};
  // scale cursor
  rect.x = window_rect.x + rect.x * window_rect.w / surface_->w;
  rect.y = window_rect.y + rect.y * window_rect.h / surface_->h;
  rect.w = rect.w * window_rect.w / surface_->w;
  rect.w *= cell.width;
  rect.h = rect.h * window_rect.h / surface_->h;
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 96);
  SDL_RenderFillRect(renderer, &rect);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderDrawRect(renderer, &rect);

  if (ringing_) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 192);
    SDL_RenderFillRect(renderer, &window_rect);
    ringing_ = 0;
  }
}

void Terminal::render_cell(VTermPos pos, const CellSurface &cellSurface,
                           int font_width, int font_height) {
  VTermScreenCell cell;
  vterm_screen_get_cell(screen_, pos, &cell);
  if (cell.chars[0] == 0xffffffff) {
    return;
  }

  // color
  SDL_Color color = {128, 128, 128};
  SDL_Color bgcolor = {0, 0, 0};
  if (VTERM_COLOR_IS_INDEXED(&cell.fg)) {
    vterm_screen_convert_color_to_rgb(screen_, &cell.fg);
  }
  if (VTERM_COLOR_IS_RGB(&cell.fg)) {
    color = {cell.fg.rgb.red, cell.fg.rgb.green, cell.fg.rgb.blue};
  }
  if (VTERM_COLOR_IS_INDEXED(&cell.bg)) {
    vterm_screen_convert_color_to_rgb(screen_, &cell.bg);
  }
  if (VTERM_COLOR_IS_RGB(&cell.bg)) {
    bgcolor = {cell.bg.rgb.red, cell.bg.rgb.green, cell.bg.rgb.blue};
  }
  if (cell.attrs.reverse) {
    std::swap(color, bgcolor);
  }

  // bg
  SDL_Rect rect = {pos.col * font_width, pos.row * font_height,
                   font_width * cell.width, font_height};
  SDL_FillRect(surface_, &rect,
               SDL_MapRGB(surface_->format, bgcolor.r, bgcolor.g, bgcolor.b));

  // fg
  if (auto text_surface = cellSurface(cell, color)) {
    SDL_SetSurfaceBlendMode(text_surface, SDL_BLENDMODE_BLEND);
    SDL_BlitSurface(text_surface, NULL, surface_, &rect);
    SDL_FreeSurface(text_surface);
  }
}

// void Terminal::processEvent(const SDL_Event &ev) 
