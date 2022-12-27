#include "vtermtest.h"
#include "vterm.h"
#include <pty.h>
#include <string.h>
#include <sys/wait.h>
#include <termios.h>
#include <unicode/normlzr.h>
#include <unicode/unistr.h>
#include <unistd.h>

Terminal::Terminal(int _fd, int _rows, int _cols, TTF_Font *_font)
    : fd_(_fd), matrix_(_rows, _cols), font_(_font),
      font_height_(TTF_FontHeight(font_)) {
  vterm_ = vterm_new(_rows, _cols);
  vterm_set_utf8(vterm_, 1);
  vterm_output_set_callback(vterm_, output_callback, (void *)&fd_);

  screen_ = vterm_obtain_screen(vterm_);
  vterm_screen_set_callbacks(screen_, &screen_callbacks, this);
  vterm_screen_reset(screen_, 1);

  matrix_.fill(0);
  TTF_SizeUTF8(font_, "X", &font_width_, NULL);
  surface_ = SDL_CreateRGBSurfaceWithFormat(
      0, font_width_ * _cols, font_height_ * _rows, 32, SDL_PIXELFORMAT_RGBA32);

  SDL_CreateRGBSurface(0, font_width_, font_height_, 32, 0, 0, 0, 0);
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
// int moverect(VTermRect dest, VTermRect src) {
//     return 0;
// }
int Terminal::movecursor(VTermPos pos, VTermPos oldpos, int visible) {
  cursor_pos_ = pos;
  return 0;
}
// int settermprop(VTermProp prop, VTermValue *val) {
//     return 0;
// }
int Terminal::bell() {
  ringing_ = true;
  return 0;
}
// int resize(int rows, int cols) {
//     return 0;
// }

// int sb_pushline(int cols, const VTermScreenCell *cells) {
//     return 0;
// }

// int sb_popline(int cols, VTermScreenCell *cells) {
//     return 0;
// }

void Terminal::render(SDL_Renderer *renderer, const SDL_Rect &window_rect) {
  if (!texture_) {
    for (int row = 0; row < matrix_.getRows(); row++) {
      for (int col = 0; col < matrix_.getCols(); col++) {
        if (matrix_(row, col)) {
          VTermPos pos = {row, col};
          render_cell(pos);
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

  SDL_Rect rect = {cursor_pos_.col * font_width_,
                   cursor_pos_.row * font_height_, font_width_, font_height_};
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

void Terminal::render_cell(VTermPos pos) {
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

  // style
  int style = TTF_STYLE_NORMAL;
  if (cell.attrs.bold)
    style |= TTF_STYLE_BOLD;
  if (cell.attrs.underline)
    style |= TTF_STYLE_UNDERLINE;
  if (cell.attrs.italic)
    style |= TTF_STYLE_ITALIC;
  if (cell.attrs.strike)
    style |= TTF_STYLE_STRIKETHROUGH;
  if (cell.attrs.blink) { /*TBD*/
  }

  // bg
  SDL_Rect rect = {pos.col * font_width_, pos.row * font_height_,
                   font_width_ * cell.width, font_height_};
  SDL_FillRect(surface_, &rect,
               SDL_MapRGB(surface_->format, bgcolor.r, bgcolor.g, bgcolor.b));

  // fg
  icu::UnicodeString ustr;
  for (int i = 0; cell.chars[i] != 0 && i < VTERM_MAX_CHARS_PER_CELL; i++) {
    ustr.append((UChar32)cell.chars[i]);
  }
  if (ustr.length() > 0) {
    UErrorCode status = U_ZERO_ERROR;
    auto normalizer = icu::Normalizer2::getNFKCInstance(status);
    if (U_FAILURE(status))
      throw std::runtime_error("unable to get NFKC normalizer");
    auto ustr_normalized = normalizer->normalize(ustr, status);
    std::string utf8;
    if (U_SUCCESS(status)) {
      ustr_normalized.toUTF8String(utf8);
    } else {
      ustr.toUTF8String(utf8);
    }
    TTF_SetFontStyle(font_, style);
    auto text_surface = TTF_RenderUTF8_Blended(font_, utf8.c_str(), color);
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

void Terminal::output_callback(const char *s, size_t len, void *user) {
  write(*(int *)user, s, len);
}

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
