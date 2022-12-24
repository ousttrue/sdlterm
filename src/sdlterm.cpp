/******************************************************************************
 * sdlterm
 ******************************************************************************
 * SDL2 Terminal Emulator
 * Based on libsdl2, libsdlfox, libvterm
 * Written in C99
 * Copyright (C) 2020 Niklas Benfer <https://github.com/palomena>
 * License: MIT License
 *****************************************************************************/

#include "sdlterm.h"
#include "sdlrenderer.h"
#include "vtermapp.h"
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <iostream>
#include <strings.h>
#include <vterm.h>

#define lengthof(f) (sizeof(f) / sizeof(f[0]))

static char clipboardbuffer[1024];

static Uint16 pixels[16 * 16] = {
    0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
    0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
    0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
    0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
    0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
    0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0aab, 0x0789, 0x0bcc, 0x0eee, 0x09aa,
    0x099a, 0x0ddd, 0x0fff, 0x0eee, 0x0899, 0x0fff, 0x0fff, 0x1fff, 0x0dde,
    0x0dee, 0x0fff, 0xabbc, 0xf779, 0x8cdd, 0x3fff, 0x9bbc, 0xaaab, 0x6fff,
    0x0fff, 0x3fff, 0xbaab, 0x0fff, 0x0fff, 0x6689, 0x6fff, 0x0dee, 0xe678,
    0xf134, 0x8abb, 0xf235, 0xf678, 0xf013, 0xf568, 0xf001, 0xd889, 0x7abc,
    0xf001, 0x0fff, 0x0fff, 0x0bcc, 0x9124, 0x5fff, 0xf124, 0xf356, 0x3eee,
    0x0fff, 0x7bbc, 0xf124, 0x0789, 0x2fff, 0xf002, 0xd789, 0xf024, 0x0fff,
    0x0fff, 0x0002, 0x0134, 0xd79a, 0x1fff, 0xf023, 0xf000, 0xf124, 0xc99a,
    0xf024, 0x0567, 0x0fff, 0xf002, 0xe678, 0xf013, 0x0fff, 0x0ddd, 0x0fff,
    0x0fff, 0xb689, 0x8abb, 0x0fff, 0x0fff, 0xf001, 0xf235, 0xf013, 0x0fff,
    0xd789, 0xf002, 0x9899, 0xf001, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0xe789,
    0xf023, 0xf000, 0xf001, 0xe456, 0x8bcc, 0xf013, 0xf002, 0xf012, 0x1767,
    0x5aaa, 0xf013, 0xf001, 0xf000, 0x0fff, 0x7fff, 0xf124, 0x0fff, 0x089a,
    0x0578, 0x0fff, 0x089a, 0x0013, 0x0245, 0x0eff, 0x0223, 0x0dde, 0x0135,
    0x0789, 0x0ddd, 0xbbbc, 0xf346, 0x0467, 0x0fff, 0x4eee, 0x3ddd, 0x0edd,
    0x0dee, 0x0fff, 0x0fff, 0x0dee, 0x0def, 0x08ab, 0x0fff, 0x7fff, 0xfabc,
    0xf356, 0x0457, 0x0467, 0x0fff, 0x0bcd, 0x4bde, 0x9bcc, 0x8dee, 0x8eff,
    0x8fff, 0x9fff, 0xadee, 0xeccd, 0xf689, 0xc357, 0x2356, 0x0356, 0x0467,
    0x0467, 0x0fff, 0x0ccd, 0x0bdd, 0x0cdd, 0x0aaa, 0x2234, 0x4135, 0x4346,
    0x5356, 0x2246, 0x0346, 0x0356, 0x0467, 0x0356, 0x0467, 0x0467, 0x0fff,
    0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
    0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
    0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
    0x0fff, 0x0fff, 0x0fff, 0x0fff};

static Uint32 TERM_GetWindowFlags(TERM_Config *cfg) {
  Uint32 flags = 0;

  static const char *names[] = {"FULLSCREEN", "BORDERLESS", "RESIZABLE",
                                "ONTOP", "MAXIMIZED"};

  static const SDL_WindowFlags values[] = {
      SDL_WINDOW_FULLSCREEN, SDL_WINDOW_BORDERLESS, SDL_WINDOW_RESIZABLE,
      SDL_WINDOW_ALWAYS_ON_TOP, SDL_WINDOW_MAXIMIZED};

  for (int i = 0; i < cfg->nWindowFlags; i++) {
    for (int k = 0; k < lengthof(values); k++) {
#ifdef MSC_VER      
      if (stricmp(cfg->windowflags[i], names[k]) == 0) 
#else
      if (strcasecmp(cfg->windowflags[i], names[k]) == 0) 
#endif
      {
        flags |= values[k];
        break;
      }
    }
  }

  return flags;
}

static int TERM_GetRendererIndex(TERM_Config *cfg) {
  int renderer_index = -1;
  if (cfg->renderer != NULL) {
    int num = SDL_GetNumRenderDrivers();
    for (int i = 0; i < num; i++) {
      SDL_RendererInfo info;
      SDL_GetRenderDriverInfo(i, &info);
      if (!strcmp(cfg->renderer, info.name)) {
        renderer_index = i;
        break;
      }
    }
  }

  return renderer_index;
}

static void swap(int *a, int *b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}
TERM_Rect TERM_Rect::FromMouseRect(const SDL_Rect &mouse_rect, int font_height,
                                   int font_width) {
  TERM_Rect rect = {
      .start_row = mouse_rect.y / font_height,
      .end_row = (mouse_rect.y + mouse_rect.h) / font_height,
      .start_col = mouse_rect.x / font_width,
      .end_col = (mouse_rect.x + mouse_rect.w) / font_width,
  };
  if (rect.start_col > rect.end_col)
    swap(&rect.start_col, &rect.end_col);
  if (rect.start_row > rect.end_row)
    swap(&rect.start_row, &rect.end_row);
  return rect;
}

SDLTermWindow::SDLTermWindow() {}

SDLTermWindow::~SDLTermWindow() {
  std::cout << "SDLTermWindow::~SDLTermWindow\n";

  renderer_ = nullptr;

  SDL_FreeSurface(this->icon_);
  SDL_FreeCursor(this->pointer_);
  SDL_DestroyWindow(this->window_);
  FOX_Exit();
}

bool SDLTermWindow::Initialize(TERM_Config *cfg, const char *title) {
  if (FOX_Init() != FOX_INITIALIZED) {
    return false;
  }

  Uint32 wflags = TERM_GetWindowFlags(cfg);
  if (wflags & SDL_WINDOW_FULLSCREEN) {
    /* Override resolution with display fullscreen resolution */
    SDL_Rect rect;
    SDL_GetDisplayBounds(0, &rect);
    cfg->width = rect.w;
    cfg->height = rect.h;
  }
  this->window_ =
      SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       cfg->width, cfg->height, wflags);
  if (!this->window_) {
    return false;
  }

  this->renderer_ =
      SDLRenderer::Create(window_, TERM_GetRendererIndex(cfg), cfg->fontpattern,
                          cfg->fontsize, cfg->boldfontpattern);
  if (!this->renderer_) {
    return false;
  }

  this->keys_ = SDL_GetKeyboardState(NULL);
  SDL_StartTextInput();

  this->pointer_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
  if (this->pointer_) {
    SDL_SetCursor(this->pointer_);
  }

  this->icon_ = SDL_CreateRGBSurfaceFrom(pixels, 16, 16, 16, 16 * 2, 0x0f00,
                                         0x00f0, 0x000f, 0xf000);
  if (this->icon_) {
    SDL_SetWindowIcon(this->window_, this->icon_);
  }

  this->mouse_rect_ = {0};
  this->mouse_down_ = false;

  this->cfg_ = *cfg;

  if (!child_.Launch(cfg->exec, cfg->args)) {
    return false;
  }

  Resize(cfg->width, cfg->height);
  renderer_->SetDirty();
  return true;
}

void SDLTermWindow::HandleWindowEvent(SDL_Event *event) {
  switch (event->window.event) {
  case SDL_WINDOWEVENT_SIZE_CHANGED:
    Resize(event->window.data1, event->window.data2);
    break;
  }
}

void SDLTermWindow::HandleKeyEvent(SDL_Event *event) {

  if (this->keys_[SDL_SCANCODE_LCTRL]) {
    int mod = SDL_toupper(event->key.keysym.sym);
    if (mod >= 'A' && mod <= 'Z') {
      char ch = mod - 'A' + 1;
      child_.Write(&ch, sizeof(ch));
      return;
    }
  }

  const char *cmd = NULL;
  switch (event->key.keysym.sym) {

  case SDLK_ESCAPE:
    cmd = "\033";
    break;

  case SDLK_LEFT:
    cmd = "\033[D";
    break;

  case SDLK_RIGHT:
    cmd = "\033[C";
    break;

  case SDLK_UP:
    cmd = "\033[A";
    break;

  case SDLK_DOWN:
    cmd = "\033[B";
    break;

  case SDLK_PAGEDOWN:
    cmd = "\033[6~";
    break;

  case SDLK_PAGEUP:
    cmd = "\033[5~";
    break;

  case SDLK_RETURN:
    cmd = "\r";
    break;

  case SDLK_INSERT:
    cmd = "\033[2~";
    break;

  case SDLK_DELETE:
    cmd = "\033[3~";
    break;

  case SDLK_BACKSPACE:
    cmd = "\b";
    break;

  case SDLK_TAB:
    cmd = "\t";
    break;

  case SDLK_F1:
    cmd = "\033OP";
    break;

  case SDLK_F2:
    cmd = "\033OQ";
    break;

  case SDLK_F3:
    cmd = "\033OR";
    break;

  case SDLK_F4:
    cmd = "\033OS";
    break;

  case SDLK_F5:
    cmd = "\033[15~";
    break;

  case SDLK_F6:
    cmd = "\033[17~";
    break;

  case SDLK_F7:
    cmd = "\033[18~";
    break;

  case SDLK_F8:
    cmd = "\033[19~";
    break;

  case SDLK_F9:
    cmd = "\033[20~";
    break;

  case SDLK_F10:
    cmd = "\033[21~";
    break;

  case SDLK_F11:
    cmd = "\033[23~";
    break;

  case SDLK_F12:
    cmd = "\033[24~";
    break;
  }

  if (cmd) {
    child_.Write(cmd, SDL_strlen(cmd));
  }
}

bool SDLTermWindow::HandleEvents() {
  SDL_Delay(20);

  SDL_Event event;
  if (child_.Closed()) {
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
  }

  child_.HandleOutputs();

  while (SDL_PollEvent(&event))
    switch (event.type) {

    case SDL_QUIT:
      return false;
      break;

    case SDL_WINDOWEVENT:
      HandleWindowEvent(&event);
      break;

    case SDL_KEYDOWN:
      HandleKeyEvent(&event);
      break;

    case SDL_TEXTINPUT:
      child_.Write(event.edit.text, SDL_strlen(event.edit.text));
      break;

    case SDL_MOUSEMOTION:
      if (this->mouse_down_) {
        this->mouse_rect_.w = event.motion.x - this->mouse_rect_.x;
        this->mouse_rect_.h = event.motion.y - this->mouse_rect_.y;
      }
      break;

    case SDL_MOUSEBUTTONDOWN:
      if (this->mouse_down_) {

      } else if (event.button.button == SDL_BUTTON_LEFT) {
        this->mouse_down_ = true;
        SDL_GetMouseState(&this->mouse_rect_.x, &this->mouse_rect_.y);
        this->mouse_rect_.w = 0;
        this->mouse_rect_.h = 0;
      }
      break;

    case SDL_MOUSEBUTTONUP:
      if (event.button.button == SDL_BUTTON_RIGHT) {
        // paste from clipboard
        char *clipboard = SDL_GetClipboardText();
        child_.Write(clipboard, SDL_strlen(clipboard));
        SDL_free(clipboard);
      } else if (event.button.button == SDL_BUTTON_LEFT) {
        // copy to clipboard
        auto rect = this->renderer_->TermRect(this->mouse_rect_);
        size_t n =
            GetTextCallback(clipboardbuffer, sizeof(clipboardbuffer), rect);
        if (n >= sizeof(clipboardbuffer)) {
          n = sizeof(clipboardbuffer) - 1;
        }
        clipboardbuffer[n] = '\0';
        SDL_SetClipboardText(clipboardbuffer);
        this->mouse_down_ = false;
      }
      break;

    case SDL_MOUSEWHEEL: {
      int size = this->cfg_.fontsize + event.wheel.y;
      renderer_->ResizeFont(size);
      Resize(this->cfg_.width, this->cfg_.height);
      this->cfg_.fontsize = size;
      break;
    }
    }

  return true;
}

void SDLTermWindow::Update() {
  auto render_screen = this->renderer_->BeginRender();
  if (render_screen) {
    for (int y = 0; y < this->cfg_.rows; y++) {
      for (int x = 0; x < this->cfg_.columns; x++) {
        CellState cell;
        if (auto ch = GetCellCallback(y, x, &cell)) {
          this->renderer_->RenderCell(x, y, ch, cell);
        }
      }
    }
  }
  this->renderer_->EndRender(render_screen, this->cfg_.width, this->cfg_.height,
                             this->mouse_down_, this->mouse_rect_);
}

void SDLTermWindow::Resize(int width, int height) {
  int cols = width / (this->renderer_->font_metrics->max_advance);
  int rows = height / this->renderer_->font_metrics->height;
  this->cfg_.width = width;
  this->cfg_.height = height;
  if (rows != this->cfg_.rows || cols != this->cfg_.columns) {
    this->cfg_.rows = rows;
    this->cfg_.columns = cols;
    this->RowsColsChanged(this->cfg_.rows, this->cfg_.columns);

    child_.NotifyTermSize(this->cfg_.rows, this->cfg_.columns);
  }
}
