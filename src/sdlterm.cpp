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
#include "vtermapp.h"
#include <cstddef>
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

/*****************************************************************************/

static Uint32 TERM_GetWindowFlags(TERM_Config *cfg);
static int TERM_GetRendererIndex(TERM_Config *cfg);
static void TERM_SignalHandler(int signum);
static void swap(int *a, int *b);

/*****************************************************************************/

Uint32 TERM_GetWindowFlags(TERM_Config *cfg) {
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

/*****************************************************************************/

int TERM_GetRendererIndex(TERM_Config *cfg) {
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
  FOX_CloseFont(this->font.bold);
  FOX_CloseFont(this->font.regular);
  SDL_FreeSurface(this->icon);
  SDL_FreeCursor(this->pointer);
  SDL_DestroyRenderer(this->renderer_);
  SDL_DestroyWindow(this->window);
  FOX_Exit();
  SDL_Quit();
}

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
      SDL_CreateRenderer(this->window, TERM_GetRendererIndex(cfg), 0);
  if (this->renderer_ == NULL) {
    SDL_DestroyWindow(this->window);
    return false;
  }

  this->keys = SDL_GetKeyboardState(NULL);
  SDL_StartTextInput();

  this->font.regular =
      FOX_OpenFont(this->renderer_, cfg->fontpattern, cfg->fontsize);
  if (this->font.regular == NULL)
    return false;

  this->font.bold =
      FOX_OpenFont(this->renderer_, cfg->boldfontpattern, cfg->fontsize);
  if (this->font.bold == NULL)
    return false;

  this->pointer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
  if (this->pointer)
    SDL_SetCursor(this->pointer);

  this->icon = SDL_CreateRGBSurfaceFrom(pixels, 16, 16, 16, 16 * 2, 0x0f00,
                                        0x00f0, 0x000f, 0xf000);
  if (this->icon)
    SDL_SetWindowIcon(this->window, this->icon);

  this->font.metrics = FOX_QueryFontMetrics(this->font.regular);
  this->ticks = SDL_GetTicks();
  this->cursor.visible = true;
  this->cursor.active = true;
  this->cursor.ticks = 0;
  this->bell.active = false;
  this->bell.ticks = 0;

  this->mouse.rect = (SDL_Rect){0};
  this->mouse.clicked = false;

  this->vterm_ = new VTermApp();
  this->vterm_->Initialize(cfg->rows, cfg->columns);

  this->vterm_->BellCallback = [state = this]() {
    state->bell.active = true;
    state->bell.ticks = state->ticks;
  };
  this->vterm_->MoveCursorCallback = [state = this](int row, int col,
                                                    bool visible) {
    state->cursor.position.x = col;
    state->cursor.position.y = row;
    if (!visible) {
      // Works great for 'top' but not for 'nano'. Nano should have a cursor!
      // state->cursor.active = false;
    } else
      state->cursor.active = true;
  };

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
  this->dirty = true;
  return true;
}

/*****************************************************************************/

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
      this->dirty = true;
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
      if (this->mouse.clicked) {
        this->mouse.rect.w = event.motion.x - this->mouse.rect.x;
        this->mouse.rect.h = event.motion.y - this->mouse.rect.y;
      }
      break;

    case SDL_MOUSEBUTTONDOWN:
      if (this->mouse.clicked) {

      } else if (event.button.button == SDL_BUTTON_LEFT) {
        this->mouse.clicked = true;
        SDL_GetMouseState(&this->mouse.rect.x, &this->mouse.rect.y);
        this->mouse.rect.w = 0;
        this->mouse.rect.h = 0;
      }
      break;

    case SDL_MOUSEBUTTONUP:
      if (event.button.button == SDL_BUTTON_RIGHT) {
        char *clipboard = SDL_GetClipboardText();
        write(this->childfd, clipboard, SDL_strlen(clipboard));
        SDL_free(clipboard);
      } else if (event.button.button == SDL_BUTTON_LEFT) {
        VTermRect rect = {
            .start_row = this->mouse.rect.y / this->font.metrics->height,
            .end_row = (this->mouse.rect.y + this->mouse.rect.h) /
                       this->font.metrics->height,
            .start_col = this->mouse.rect.x / this->font.metrics->max_advance,
            .end_col = (this->mouse.rect.x + this->mouse.rect.w) /
                       this->font.metrics->max_advance,
        };
        if (rect.start_col > rect.end_col)
          swap(&rect.start_col, &rect.end_col);
        if (rect.start_row > rect.end_row)
          swap(&rect.start_row, &rect.end_row);

        size_t n =
            vterm_->GetText(clipboardbuffer, sizeof(clipboardbuffer), rect);

        if (n >= sizeof(clipboardbuffer))
          n = sizeof(clipboardbuffer) - 1;
        clipboardbuffer[n] = '\0';
        SDL_SetClipboardText(clipboardbuffer);
        this->mouse.clicked = false;
      }
      break;

    case SDL_MOUSEWHEEL: {
      int size = this->cfg.fontsize;
      size += event.wheel.y;
      FOX_CloseFont(this->font.regular);
      FOX_CloseFont(this->font.bold);
      this->font.regular =
          FOX_OpenFont(this->renderer_, this->cfg.fontpattern, size);
      if (this->font.regular == NULL)
        return -1;

      this->font.bold =
          FOX_OpenFont(this->renderer_, this->cfg.boldfontpattern, size);
      if (this->font.bold == NULL)
        return -1;
      this->font.metrics = FOX_QueryFontMetrics(this->font.regular);
      Resize(this->cfg.width, this->cfg.height);
      this->cfg.fontsize = size;
      break;
    }
    }

  return status == 0;
}

/*****************************************************************************/

void SDLApp::Update() {
  this->ticks = SDL_GetTicks();

  if (this->dirty) {
    SDL_SetRenderDrawColor(this->renderer_, 0, 0, 0, 255);
    SDL_RenderClear(this->renderer_);
    RenderScreen();
  }

  if (this->ticks > (this->cursor.ticks + 250)) {
    this->cursor.ticks = this->ticks;
    this->cursor.visible = !this->cursor.visible;
    this->dirty = true;
  }

  if (this->bell.active && (this->ticks > (this->bell.ticks + 250))) {
    this->bell.active = false;
  }

  if (this->mouse.clicked) {
    SDL_RenderDrawRect(this->renderer_, &this->mouse.rect);
  }

  SDL_RenderPresent(this->renderer_);
}

/*****************************************************************************/

void SDLApp::RenderCursor() {
  if (this->cursor.active && this->cursor.visible) {
    SDL_Rect rect = {this->cursor.position.x * this->font.metrics->max_advance,
                     4 + this->cursor.position.y * this->font.metrics->height,
                     4, this->font.metrics->height};
    SDL_RenderFillRect(this->renderer_, &rect);
  }
}

void SDLApp::RenderScreen() {
  SDL_SetRenderDrawColor(this->renderer_, 255, 255, 255, 255);
  for (unsigned y = 0; y < this->cfg.rows; y++) {
    for (unsigned x = 0; x < this->cfg.columns; x++) {
      this->RenderCell(x, y);
    }
  }

  RenderCursor();
  this->dirty = false;

  if (this->bell.active) {
    SDL_Rect rect = {0, 0, this->cfg.width, this->cfg.height};
    SDL_RenderDrawRect(this->renderer_, &rect);
  }
}

/*****************************************************************************/

void SDLApp::RenderCell(int x, int y) {
  FOX_Font *font = this->font.regular;
  VTermPos pos = {.row = y, .col = x};
  SDL_Point cursor = {x * this->font.metrics->max_advance,
                      y * this->font.metrics->height};

  auto cell = this->vterm_->GetCell(pos);
  Uint32 ch = cell->chars[0];
  if (ch == 0)
    return;

  this->vterm_->UpdateCell(cell);
  SDL_Color color = {cell->fg.rgb.red, cell->fg.rgb.green, cell->fg.rgb.blue,
                     255};
  if (cell->attrs.reverse) {
    SDL_Rect rect = {cursor.x, cursor.y + 4, this->font.metrics->max_advance,
                     this->font.metrics->height};
    SDL_SetRenderDrawColor(this->renderer_, color.r, color.g, color.b, color.a);
    color.r = ~color.r;
    color.g = ~color.g;
    color.b = ~color.b;
    SDL_RenderFillRect(this->renderer_, &rect);
  }

  if (cell->attrs.bold)
    font = this->font.bold;
  else if (cell->attrs.italic)
    ;

  SDL_SetRenderDrawColor(this->renderer_, color.r, color.g, color.b, color.a);
  FOX_RenderChar(font, ch, 0, &cursor);
}

void SDLApp::Resize(int width, int height) {
  int cols = width / (this->font.metrics->max_advance);
  int rows = height / this->font.metrics->height;
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

void TERM_SignalHandler(int signum) { childState = 0; }

void swap(int *a, int *b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}
