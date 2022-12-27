#include "SDL_pixels.h"
#include "SDL_surface.h"
#include "vterm.h"
#include "vtermtest.h"
#include <iostream>
#include <string.h>
#include <unicode/normlzr.h>
#include <unicode/unistr.h>

#include <childprocess.h>
#include <sdl_app.h>

auto FONT_FILE = "/usr/share/fonts/vlgothic/VL-Gothic-Regular.ttf";

static std::string cp2utf8(const icu::UnicodeString &ustr) {
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
  return utf8;
}

int main(int argc, char **argv) {
  if (argc > 1) {
    FONT_FILE = argv[1];
  }

  termtk::SDLApp app;

  if (TTF_Init() < 0) {
    std::cerr << "TTF_Init: " << TTF_GetError() << std::endl;
    return 2;
  }

  TTF_Font *font = TTF_OpenFont(FONT_FILE, 48);
  if (font == NULL) {
    std::cerr << "TTF_OpenFont: " << TTF_GetError() << std::endl;
    return 3;
  }
  int font_height = TTF_FontHeight(font);
  int font_width;
  TTF_SizeUTF8(font, "X", &font_width, NULL);

  auto window = app.CreateWindow(1024, 768, "term");
  if (window == NULL) {
    std::cerr << "SDL_CreateWindow: " << SDL_GetError() << std::endl;
    return 4;
  }

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window->Handle(), -1, SDL_RENDERER_PRESENTVSYNC);
  if (renderer == NULL) {
    std::cerr << "SDL_CreateRenderer: " << SDL_GetError() << std::endl;
    return 5;
  }

  const int rows = 32;
  const int cols = 100;

  termtk::ChildProcess child;
  child.Launch(rows, cols, getenv("SHELL"), {"-"});

  Terminal terminal(rows, cols, font_width, font_height,
                    &termtk::ChildProcess::Write, &child);

  auto cellSurface = [font](const VTermScreenCell &cell,
                            SDL_Color color) -> SDL_Surface * {
    // code points to utf8
    icu::UnicodeString ustr;
    for (int i = 0; cell.chars[i] != 0 && i < VTERM_MAX_CHARS_PER_CELL; i++) {
      ustr.append((UChar32)cell.chars[i]);
    }
    if (ustr.length() == 0) {
      return nullptr;
    }
    auto utf8 = cp2utf8(ustr);

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

    TTF_SetFontStyle(font, style);
    return TTF_RenderUTF8_Blended(font, utf8.c_str(), color);
  };

  while (!child.IsClosed()) {
    // child output
    auto input = child.Read();
    if (!input.empty()) {
      terminal.input_write(input.data(), input.size());
    }

    // window event
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_QUIT ||
          (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE &&
           (ev.key.keysym.mod & KMOD_CTRL))) {
        child.Kill();
      } else {
        // terminal.processEvent(ev);
        {
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
              terminal.keyboard_unichar(ev.text.text[i], (VTermModifier)mod);
            }
          } else if (ev.type == SDL_KEYDOWN) {
            switch (ev.key.keysym.sym) {
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
              terminal.keyboard_key(VTERM_KEY_ENTER, VTERM_MOD_NONE);
              break;
            case SDLK_BACKSPACE:
              terminal.keyboard_key(VTERM_KEY_BACKSPACE, VTERM_MOD_NONE);
              break;
            case SDLK_ESCAPE:
              terminal.keyboard_key(VTERM_KEY_ESCAPE, VTERM_MOD_NONE);
              break;
            case SDLK_TAB:
              terminal.keyboard_key(VTERM_KEY_TAB, VTERM_MOD_NONE);
              break;
            case SDLK_UP:
              terminal.keyboard_key(VTERM_KEY_UP, VTERM_MOD_NONE);
              break;
            case SDLK_DOWN:
              terminal.keyboard_key(VTERM_KEY_DOWN, VTERM_MOD_NONE);
              break;
            case SDLK_LEFT:
              terminal.keyboard_key(VTERM_KEY_LEFT, VTERM_MOD_NONE);
              break;
            case SDLK_RIGHT:
              terminal.keyboard_key(VTERM_KEY_RIGHT, VTERM_MOD_NONE);
              break;
            case SDLK_PAGEUP:
              terminal.keyboard_key(VTERM_KEY_PAGEUP, VTERM_MOD_NONE);
              break;
            case SDLK_PAGEDOWN:
              terminal.keyboard_key(VTERM_KEY_PAGEDOWN, VTERM_MOD_NONE);
              break;
            case SDLK_HOME:
              terminal.keyboard_key(VTERM_KEY_HOME, VTERM_MOD_NONE);
              break;
            case SDLK_END:
              terminal.keyboard_key(VTERM_KEY_END, VTERM_MOD_NONE);
              break;
            default:
              if (ev.key.keysym.mod & KMOD_CTRL && ev.key.keysym.sym < 127) {
                // std::cout << ev.key.keysym.sym << std::endl;
                terminal.keyboard_unichar(ev.key.keysym.sym, VTERM_MOD_CTRL);
              }
              break;
            }
          }
        }
      }
    }

    // render
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_Rect rect = {0, 0, 1024, 768};
    terminal.render(renderer, rect, cellSurface, font_width, font_height);
    SDL_RenderPresent(renderer);
  }

  TTF_Quit();
  return 0;
}
