#include "sdl_app.h"
#include "SDL_stdinc.h"
#include "SDL_video.h"
#include <SDL.h>
#include <SDL_fox.h>
#include <compare>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_map>

#include "icon.h"

namespace termtk {

//
// SDLWindow
//
struct SDLWindowImpl {
  SDL_Window *window_;
  SDL_Surface *icon_;
  int width_ = 0;
  int height_ = 0;

  SDLWindowImpl(SDL_Window *window) : window_(window) {
    SDL_GetWindowSize(window_, &width_, &height_);

    icon_ = SDL_CreateRGBSurfaceFrom(pixels, 16, 16, 16, 16 * 2, 0x0f00, 0x00f0,
                                     0x000f, 0xf000);
    if (icon_) {
      SDL_SetWindowIcon(this->window_, this->icon_);
    }
  }
  ~SDLWindowImpl() {
    SDL_FreeSurface(this->icon_);
    SDL_DestroyWindow(window_);
  }
};

SDLWindow::SDLWindow(SDL_Window *window) : impl_(new SDLWindowImpl(window)) {}
SDLWindow::~SDLWindow() { delete impl_; }
int SDLWindow::Width() const { return impl_->width_; }
int SDLWindow::Height() const { return impl_->height_; }
struct SDL_Window *SDLWindow::Handle() const { return impl_->window_; }

//
// SDLApp
//
struct SDLAppImpl {
  SDL_Cursor *pointer_;
  const Uint8 *keys_;
  std::vector<char> keyInputBuffer_;
  std::vector<char> tmp_;
  std::unordered_map<Uint32, std::weak_ptr<SDLWindow>> windowMap_;

  SDLAppImpl() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
      throw std::runtime_error("SDL_Init");
    }
    if (FOX_Init() != FOX_INITIALIZED) {
      throw std::runtime_error("FOX_Init");
    }

    // SDL_ShowCursor(SDL_DISABLE);
    this->pointer_ = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
    if (this->pointer_) {
      SDL_SetCursor(this->pointer_);
    }

    this->keys_ = SDL_GetKeyboardState(NULL);
    SDL_StartTextInput();
  }

  ~SDLAppImpl() {
    std::cout << "termtk::SDLAppImpl::~SDLAppImpl\n";
    SDL_FreeCursor(this->pointer_);

    FOX_Exit();
    SDL_Quit();
  }

  std::span<char> DequeueInput() {
    std::swap(tmp_, keyInputBuffer_);
    keyInputBuffer_.clear();
    return {tmp_.data(), tmp_.size()};
  }

  bool NewFrame() {
    SDL_Delay(20);

    SDL_Event event;
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
        for (auto p = event.edit.text; *p; ++p) {
          keyInputBuffer_.push_back(*p);
        }
        break;
      }

    return true;
  }

private:
  void HandleWindowEvent(SDL_Event *event) {
    switch (event->window.event) {
    case SDL_WINDOWEVENT_SIZE_CHANGED:
      auto found = windowMap_.find(event->window.windowID);
      if (found != windowMap_.end()) {
        auto window = found->second.lock();
        window->impl_->width_ = event->window.data1;
        window->impl_->height_ = event->window.data2;
      }
      break;
    }
  }

  void HandleKeyEvent(SDL_Event *event) {

    if (this->keys_[SDL_SCANCODE_LCTRL]) {
      int mod = SDL_toupper(event->key.keysym.sym);
      if (mod >= 'A' && mod <= 'Z') {
        char ch = mod - 'A' + 1;
        keyInputBuffer_.push_back(ch);
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
      for (auto p = cmd; *p; ++p) {
        keyInputBuffer_.push_back(*p);
      }
    }
  }
};

SDLApp::SDLApp() : impl_(new SDLAppImpl) {}
SDLApp::~SDLApp() { delete impl_; }
std::shared_ptr<SDLWindow> SDLApp::CreateWindow(int width, int height,
                                                const char *title) {

  auto window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED, width, height, 0);
  if (!window) {
    return nullptr;
  }

  auto ptr = std::shared_ptr<SDLWindow>(new SDLWindow(window));
  return ptr;
}
bool SDLApp::NewFrame() { return impl_->NewFrame(); }
std::span<char> SDLApp::DequeueInput() { return impl_->DequeueInput(); }

} // namespace termtk
