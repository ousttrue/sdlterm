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
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include "TERM_Rect.h"
#include "sdlrenderer.h"
#include "vtermapp.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <pty.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vterm.h>

/*****************************************************************************/

#define lengthof(f) (sizeof(f) / sizeof(f[0]))

static int childState = 0;

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
      if (!strcasecmp(cfg->windowflags[i], names[k])) {
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

SDLApp::SDLApp() {}

SDLApp::~SDLApp() {
  delete vterm_;

  kill(this->child, SIGKILL);
  pid_t wpid;
  int wstatus;
  do {
    wpid = waitpid(this->child, &wstatus, WUNTRACED | WCONTINUED);
    if (wpid == -1)
      break;
  } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
  this->child = wpid;
  SDL_FreeSurface(this->icon);
  SDL_FreeCursor(this->pointer);
  renderer_ = nullptr;
  SDL_DestroyWindow(this->window);
  FOX_Exit();
  SDL_Quit();
}

static void TERM_SignalHandler(int signum) { childState = 0; }

bool SDLApp::Initialize(TERM_Config *cfg, const char *title) {
  if (SDL_Init(SDL_INIT_VIDEO)) {
    return false;
  }

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
  this->window =
      SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       cfg->width, cfg->height, wflags);
  if (this->window == NULL) {
    return false;
  }

  this->renderer_ =
      SDLRenderer::Create(window, TERM_GetRendererIndex(cfg), cfg->fontpattern,
                          cfg->fontsize, cfg->boldfontpattern);
  if (this->renderer_ == NULL) {
    return false;
  }

  this->keys = SDL_GetKeyboardState(NULL);
  SDL_StartTextInput();

  this->pointer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
  if (this->pointer)
    SDL_SetCursor(this->pointer);

  this->icon = SDL_CreateRGBSurfaceFrom(pixels, 16, 16, 16, 16 * 2, 0x0f00,
                                        0x00f0, 0x000f, 0xf000);
  if (this->icon)
    SDL_SetWindowIcon(this->window, this->icon);

  this->mouse_rect = (SDL_Rect){0};
  this->mouse_clicked = false;

  this->vterm_ = new VTermApp();
  this->vterm_->Initialize(cfg->rows, cfg->columns);

  this->vterm_->BellCallback =
      std::bind(&SDLRenderer::SetBell, this->renderer_);
  this->vterm_->MoveCursorCallback = std::bind(
      &SDLRenderer::MoveCursor, this->renderer_, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3);

  this->cfg = *cfg;

  this->child = forkpty(&this->childfd, NULL, NULL, NULL);
  if (this->child < 0)
    return false;
  else if (this->child == 0) {
    execvp(cfg->exec, cfg->args);
    exit(0);
  } else {
    struct sigaction action = {0};
    action.sa_handler = TERM_SignalHandler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGCHLD, &action, NULL);
    childState = 1;
  }

  Resize(cfg->width, cfg->height);
  renderer_->SetDirty();
  return true;
}

void SDLApp::HandleWindowEvent(SDL_Event *event) {
  switch (event->window.event) {
  case SDL_WINDOWEVENT_SIZE_CHANGED:
    Resize(event->window.data1, event->window.data2);
    break;
  }
}

void SDLApp::HandleKeyEvent(SDL_Event *event) {

  if (this->keys[SDL_SCANCODE_LCTRL]) {
    int mod = SDL_toupper(event->key.keysym.sym);
    if (mod >= 'A' && mod <= 'Z') {
      char ch = mod - 'A' + 1;
      write(this->childfd, &ch, sizeof(ch));
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
    write(this->childfd, cmd, SDL_strlen(cmd));
  }
}

void SDLApp::HandleChildEvents() {
  fd_set rfds;
  struct timeval tv = {0};

  FD_ZERO(&rfds);
  FD_SET(this->childfd, &rfds);

  tv.tv_sec = 0;
  tv.tv_usec = 50000;

  if (select(this->childfd + 1, &rfds, NULL, NULL, &tv) > 0) {
    char line[256];
    int n;
    if ((n = read(this->childfd, line, sizeof(line))) > 0) {
      this->vterm_->Write(line, n);
      this->renderer_->SetDirty();
      // vterm_screen_flush_damage(this->screen);
    }
  }
}

bool SDLApp::HandleEvents() {
  SDL_Delay(20);

  SDL_Event event;
  if (childState == 0) {
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
  }

  HandleChildEvents();

  int status = 0;
  while (SDL_PollEvent(&event))
    switch (event.type) {

    case SDL_QUIT:
      status = 1;
      break;

    case SDL_WINDOWEVENT:
      HandleWindowEvent(&event);
      break;

    case SDL_KEYDOWN:
      HandleKeyEvent(&event);
      break;

    case SDL_TEXTINPUT:
      write(this->childfd, event.edit.text, SDL_strlen(event.edit.text));
      break;

    case SDL_MOUSEMOTION:
      if (this->mouse_clicked) {
        this->mouse_rect.w = event.motion.x - this->mouse_rect.x;
        this->mouse_rect.h = event.motion.y - this->mouse_rect.y;
      }
      break;

    case SDL_MOUSEBUTTONDOWN:
      if (this->mouse_clicked) {

      } else if (event.button.button == SDL_BUTTON_LEFT) {
        this->mouse_clicked = true;
        SDL_GetMouseState(&this->mouse_rect.x, &this->mouse_rect.y);
        this->mouse_rect.w = 0;
        this->mouse_rect.h = 0;
      }
      break;

    case SDL_MOUSEBUTTONUP:
      if (event.button.button == SDL_BUTTON_RIGHT) {
        char *clipboard = SDL_GetClipboardText();
        write(this->childfd, clipboard, SDL_strlen(clipboard));
        SDL_free(clipboard);
      } else if (event.button.button == SDL_BUTTON_LEFT) {
        auto rect = TERM_Rect::FromMouseRect(
            this->mouse_rect, this->renderer_->font_metrics->height,
            this->renderer_->font_metrics->max_advance);

        size_t n =
            vterm_->GetText(clipboardbuffer, sizeof(clipboardbuffer), rect);

        if (n >= sizeof(clipboardbuffer))
          n = sizeof(clipboardbuffer) - 1;
        clipboardbuffer[n] = '\0';
        SDL_SetClipboardText(clipboardbuffer);
        this->mouse_clicked = false;
      }
      break;

    case SDL_MOUSEWHEEL: {
      int size = this->cfg.fontsize + event.wheel.y;
      renderer_->ResizeFont(size);
      Resize(this->cfg.width, this->cfg.height);
      this->cfg.fontsize = size;
      break;
    }
    }

  return status == 0;
}

void SDLApp::Update() {
  auto render_screen = this->renderer_->BeginRender();
  if (render_screen) {
    for (int y = 0; y < this->cfg.rows; y++) {
      for (int x = 0; x < this->cfg.columns; x++) {
        VTermPos pos = {
            .row = y,
            .col = x,
        };
        auto cell = this->vterm_->GetCell(pos);
        Uint32 ch = cell->chars[0];
        if (ch == 0)
          continue;
        ;
        this->vterm_->UpdateCell(cell);
        SDL_Color color = {cell->fg.rgb.red, cell->fg.rgb.green,
                           cell->fg.rgb.blue, 255};
        this->renderer_->RenderCell(x, y, ch, color, cell->attrs.reverse,
                                    cell->attrs.bold, cell->attrs.italic);
      }
    }
  }
  this->renderer_->EndRender(render_screen, this->cfg.width, this->cfg.height,
                             this->mouse_clicked, this->mouse_rect);
}

void SDLApp::Resize(int width, int height) {
  int cols = width / (this->renderer_->font_metrics->max_advance);
  int rows = height / this->renderer_->font_metrics->height;
  this->cfg.width = width;
  this->cfg.height = height;
  if (rows != this->cfg.rows || cols != this->cfg.columns) {
    this->cfg.rows = rows;
    this->cfg.columns = cols;
    this->vterm_->Resize(this->cfg.rows, this->cfg.columns);

    struct winsize ws = {0};
    ws.ws_col = this->cfg.columns;
    ws.ws_row = this->cfg.rows;
    ioctl(this->childfd, TIOCSWINSZ, &ws);
  }
}
