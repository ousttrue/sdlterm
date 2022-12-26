#include "sdlrenderer.h"
#include "SDL_pixels.h"
#include "vterm.h"
#include <iostream>

SDLRenderer::SDLRenderer(SDL_Renderer *renderer) : renderer_(renderer) {}
SDLRenderer::~SDLRenderer() {
  std::cout << "SDLRenderer::~SDLRenderer\n";
  if (font_bold) {
    FOX_CloseFont(this->font_bold);
  }
  if (font_regular) {
    FOX_CloseFont(this->font_regular);
  }
  SDL_DestroyRenderer(this->renderer_);
}
std::shared_ptr<SDLRenderer> SDLRenderer::Create(SDL_Window *window) {
  auto renderer = SDL_CreateRenderer(window, -1, 0);
  if (!renderer) {
    return nullptr;
  }

  auto ptr = std::shared_ptr<SDLRenderer>(new SDLRenderer(renderer));
  ptr->ticks = SDL_GetTicks();
  return ptr;
}

bool SDLRenderer::LoadFont(const char *fontpattern, int fontsize,
                           const char *boldfontpattern) {
  this->font_regular = FOX_OpenFont(this->renderer_, fontpattern, fontsize);
  if (!this->font_regular) {
    return false;
  }
  this->fontpattern = fontpattern;
  this->font_metrics = FOX_QueryFontMetrics(this->font_regular);

  this->font_bold = FOX_OpenFont(this->renderer_, boldfontpattern, fontsize);
  if (!this->font_bold) {
    return false;
  }
  this->boldfontpattern = boldfontpattern;

  return true;
}

bool SDLRenderer::ResizeFont(int size) {
  FOX_CloseFont(this->font_regular);
  FOX_CloseFont(this->font_bold);
  this->font_regular =
      FOX_OpenFont(this->renderer_, this->fontpattern.c_str(), size);
  if (!this->font_regular) {
    return false;
  }

  this->font_bold =
      FOX_OpenFont(this->renderer_, this->boldfontpattern.c_str(), size);
  if (!this->font_bold) {
    return false;
  }
  this->font_metrics = FOX_QueryFontMetrics(this->font_regular);
  return true;
  ;
}

bool SDLRenderer::BeginRender() {

  this->ticks = SDL_GetTicks();

  auto screen_render = this->dirty;
  if (this->dirty) {
    SDL_SetRenderDrawColor(this->renderer_, 0, 0, 0, 255);
    SDL_RenderClear(this->renderer_);
    SDL_SetRenderDrawColor(this->renderer_, 255, 255, 255, 255);

    // RenderScreen(rows, cols, width, height);
  }

  if (this->ticks > (this->cursor.ticks + 250)) {
    this->cursor.ticks = this->ticks;
    this->cursor.visible = !this->cursor.visible;
    this->dirty = true;
  }

  if (this->bell.active && (this->ticks > (this->bell.ticks + 250))) {
    this->bell.active = false;
  }

  return screen_render;
}

void SDLRenderer::EndRender(bool screen_render, int width, int height) {
  if (screen_render) {
    RenderCursor();
    this->dirty = false;

    if (this->bell.active) {
      SDL_Rect rect = {0, 0, width, height};
      SDL_RenderDrawRect(this->renderer_, &rect);
    }
  }

  // if (mouse_clicked) {
  //   SDL_RenderDrawRect(this->renderer_, &mouse_rect);
  // }

  SDL_RenderPresent(this->renderer_);
}

void SDLRenderer::RenderCursor() {
  if (this->cursor.active && this->cursor.visible) {
    SDL_Rect rect = {this->cursor.position.x * this->font_metrics->max_advance,
                     4 + this->cursor.position.y * this->font_metrics->height,
                     4, this->font_metrics->height};
    SDL_RenderFillRect(this->renderer_, &rect);
  }
}

void SDLRenderer::RenderCell(const VTermPos &pos, const VTermScreenCell &cell) {
  FOX_Font *font = this->font_regular;
  SDL_Point cursor = {pos.col * this->font_metrics->max_advance,
                      pos.row * this->font_metrics->height};

  SDL_Color fg = {
      .r = cell.fg.rgb.red,
      .g = cell.fg.rgb.green,
      .b = cell.fg.rgb.blue,
      .a = 255,
  };
  SDL_Color bg = {
      .r = cell.bg.rgb.red,
      .g = cell.bg.rgb.green,
      .b = cell.bg.rgb.blue,
      .a = 255,
  };
  if (cell.attrs.reverse) {
    // auto tmp = fg;
    // fg = bg;
    // bg = tmp;
    fg.r = ~fg.r;
    fg.g = ~fg.g;
    fg.b = ~fg.b;
    bg.r = ~bg.r;
    bg.g = ~bg.g;
    bg.b = ~bg.b;
  }

  // BG
  SDL_Rect rect = {cursor.x, cursor.y + 4, this->font_metrics->max_advance,
                   this->font_metrics->height};
  SDL_SetRenderDrawColor(this->renderer_, bg.r, bg.g, bg.b, bg.a);
  SDL_RenderFillRect(this->renderer_, &rect);

  // FG
  if (cell.attrs.bold) {
    font = this->font_bold;
  } else if (cell.attrs.italic) {
  }
  if (auto ch = cell.chars[0]) {
    SDL_SetRenderDrawColor(this->renderer_, fg.r, fg.g, fg.b, fg.a);
    FOX_RenderChar(font, ch, 0, &cursor);
  }
}
// return &cell;
// Uint32 ch = cell.chars[0];
// if (ch) {
//   vterm_state_convert_color_to_rgb(termstate, &cell.fg);
//   vterm_state_convert_color_to_rgb(termstate, &cell.bg);
// }
// return ch;
