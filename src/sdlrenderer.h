#pragma once
#include "SDL_pixels.h"
#include "SDL_rect.h"
#include "TERM_Rect.h"
#include <SDL.h>
#include <SDL_fox.h>
#include <memory>
#include <string>

struct CellState {
  SDL_Color color;
  bool attrs_reverse;
  bool attrs_bold;
  bool attrs_italic;
};

class SDLRenderer {
  SDL_Renderer *renderer_;

  bool dirty;

  std::string fontpattern;
  FOX_Font *font_regular;
  std::string boldfontpattern;
  FOX_Font *font_bold;
  Uint32 ticks;
  struct {
    Uint32 ticks;
    bool active;
  } bell;

  SDLRenderer(SDL_Renderer *renderer);

public:
  const FOX_FontMetrics *font_metrics;
  struct {
    SDL_Point position;
    bool visible;
    bool active;
    Uint32 ticks;
  } cursor;

  ~SDLRenderer();
  static std::shared_ptr<SDLRenderer> Create(SDL_Window *window, int index,
                                             const char *fontpattern,
                                             int fontsize,
                                             const char *boldfontpattern);
  void SetDirty() { this->dirty = true; }
  bool ResizeFont(int d);
  bool BeginRender();
  void EndRender(bool render_screen, int width, int height, bool mouse_clicked,
                 const SDL_Rect &mouse_rect);
  void SetBell() {
    bell.active = true;
    bell.ticks = ticks;
  }
  void MoveCursor(int row, int col, bool visible) {
    cursor.position.x = col;
    cursor.position.y = row;
    if (!visible) {
      // Works great for 'top' but not for 'nano'. Nano should have a
      // cursor! state->cursor.active = false;
    } else {
      cursor.active = true;
    }
  }
  void RenderCell(int x, int y, uint32_t ch, CellState cell);
  TERM_Rect TermRect(const SDL_Rect &rect) const {
    return TERM_Rect::FromMouseRect(rect, this->font_metrics->height,
                                    this->font_metrics->max_advance);
  }

private:
  void RenderCursor();
};
