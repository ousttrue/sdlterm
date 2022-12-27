#include "vtermtest.h"
#include "SDL_video.h"
#include "vterm.h"
#include <iostream>
#include <pty.h>
#include <string.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

static int damage(VTermRect rect, void *user) {
  return ((Terminal *)user)
      ->damage(rect.start_row, rect.start_col, rect.end_row, rect.end_col);
}

static int moverect(VTermRect dest, VTermRect src, void *user) {
  return ((Terminal *)user)->moverect(dest, src);
}

static int movecursor(VTermPos pos, VTermPos oldpos, int visible, void *user) {
  return ((Terminal *)user)->movecursor(pos, oldpos, visible);
}

static int settermprop(VTermProp prop, VTermValue *val, void *user) {
  return ((Terminal *)user)->settermprop(prop, val);
}

static int bell(void *user) { return ((Terminal *)user)->bell(); }

static int resize(int rows, int cols, void *user) {
  return ((Terminal *)user)->resize(rows, cols);
}

static int sb_pushline(int cols, const VTermScreenCell *cells, void *user) {
  return ((Terminal *)user)->sb_pushline(cols, cells);
}

static int sb_popline(int cols, VTermScreenCell *cells, void *user) {
  return ((Terminal *)user)->sb_popline(cols, cells);
}

const VTermScreenCallbacks screen_callbacks = {
    damage, moverect, movecursor,  settermprop,
    bell,   resize,   sb_pushline, sb_popline};

static void output_callback(const char *s, size_t len, void *user) {
  write(*(int *)user, s, len);
}

Terminal::Terminal(int _fd, int _rows, int _cols, int font_width,
                   int font_height)
    : fd_(_fd), matrix_(_rows, _cols) {
  vterm_ = vterm_new(_rows, _cols);
  vterm_set_utf8(vterm_, 1);
  vterm_output_set_callback(vterm_, output_callback, (void *)&fd_);

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
    std::cout << "VTERM_PROP_TITLE: " << val->string.str << std::endl;
    break;
  case VTERM_PROP_ICONNAME:
    // string
    std::cout << "VTERM_PROP_ICONNAME: " << val->string.str << std::endl;
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
  SDL_Color color = (SDL_Color){128, 128, 128};
  SDL_Color bgcolor = (SDL_Color){0, 0, 0};
  if (VTERM_COLOR_IS_INDEXED(&cell.fg)) {
    vterm_screen_convert_color_to_rgb(screen_, &cell.fg);
  }
  if (VTERM_COLOR_IS_RGB(&cell.fg)) {
    color = (SDL_Color){cell.fg.rgb.red, cell.fg.rgb.green, cell.fg.rgb.blue};
  }
  if (VTERM_COLOR_IS_INDEXED(&cell.bg)) {
    vterm_screen_convert_color_to_rgb(screen_, &cell.bg);
  }
  if (VTERM_COLOR_IS_RGB(&cell.bg)) {
    bgcolor = (SDL_Color){cell.bg.rgb.red, cell.bg.rgb.green, cell.bg.rgb.blue};
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

void Terminal::processEvent(const SDL_Event &ev) {
  if (ev.type == SDL_TEXTINPUT) {
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    int mod = VTERM_MOD_NONE;
    if (state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL])
      mod |= VTERM_MOD_CTRL;
    if (state[SDL_SCANCODE_LALT] || state[SDL_SCANCODE_RALT])
      mod |= VTERM_MOD_ALT;
    if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT])
      mod |= VTERM_MOD_SHIFT;
    for (int i = 0; i < strlen(ev.text.text); i++) {
      keyboard_unichar(ev.text.text[i], (VTermModifier)mod);
    }
  } else if (ev.type == SDL_KEYDOWN) {
    switch (ev.key.keysym.sym) {
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
      keyboard_key(VTERM_KEY_ENTER, VTERM_MOD_NONE);
      break;
    case SDLK_BACKSPACE:
      keyboard_key(VTERM_KEY_BACKSPACE, VTERM_MOD_NONE);
      break;
    case SDLK_ESCAPE:
      keyboard_key(VTERM_KEY_ESCAPE, VTERM_MOD_NONE);
      break;
    case SDLK_TAB:
      keyboard_key(VTERM_KEY_TAB, VTERM_MOD_NONE);
      break;
    case SDLK_UP:
      keyboard_key(VTERM_KEY_UP, VTERM_MOD_NONE);
      break;
    case SDLK_DOWN:
      keyboard_key(VTERM_KEY_DOWN, VTERM_MOD_NONE);
      break;
    case SDLK_LEFT:
      keyboard_key(VTERM_KEY_LEFT, VTERM_MOD_NONE);
      break;
    case SDLK_RIGHT:
      keyboard_key(VTERM_KEY_RIGHT, VTERM_MOD_NONE);
      break;
    case SDLK_PAGEUP:
      keyboard_key(VTERM_KEY_PAGEUP, VTERM_MOD_NONE);
      break;
    case SDLK_PAGEDOWN:
      keyboard_key(VTERM_KEY_PAGEDOWN, VTERM_MOD_NONE);
      break;
    case SDLK_HOME:
      keyboard_key(VTERM_KEY_HOME, VTERM_MOD_NONE);
      break;
    case SDLK_END:
      keyboard_key(VTERM_KEY_END, VTERM_MOD_NONE);
      break;
    default:
      if (ev.key.keysym.mod & KMOD_CTRL && ev.key.keysym.sym < 127) {
        // std::cout << ev.key.keysym.sym << std::endl;
        keyboard_unichar(ev.key.keysym.sym, VTERM_MOD_CTRL);
      }
      break;
    }
  }
}

void Terminal::processInput() {
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(fd_, &readfds);
  timeval timeout = {0, 0};
  if (select(fd_ + 1, &readfds, NULL, NULL, &timeout) > 0) {
    char buf[4096];
    auto size = read(fd_, buf, sizeof(buf));
    if (size > 0) {
      input_write(buf, size);
    }
  }
}

std::pair<int, int>
createSubprocessWithPty(int rows, int cols, const char *prog,
                        const std::vector<std::string> &args,
                        const char *TERM) {
  int fd;
  struct winsize win = {(unsigned short)rows, (unsigned short)cols, 0, 0};
  auto pid = forkpty(&fd, NULL, NULL, &win);
  if (pid < 0)
    throw std::runtime_error("forkpty failed");
  // else
  if (!pid) {
    setenv("TERM", TERM, 1);
    char **argv = new char *[args.size() + 2];
    argv[0] = strdup(prog);
    for (int i = 1; i <= args.size(); i++) {
      argv[i] = strdup(args[i - 1].c_str());
    }
    argv[args.size() + 1] = NULL;
    if (execvp(prog, argv) < 0)
      exit(-1);
  }
  // else
  return {pid, fd};
}

std::pair<pid_t, int> waitpid(pid_t pid, int options) {
  int status;
  auto done_pid = waitpid(pid, &status, options);
  return {done_pid, status};
}
