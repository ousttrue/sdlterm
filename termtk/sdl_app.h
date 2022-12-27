#pragma once
#include "SDL_video.h"
#include <memory>
#include <span>
#include <vector>

namespace termtk {

class SDLWindow {
  friend class SDLAppImpl;
  class SDLWindowImpl *impl_ = nullptr;

public:
  SDLWindow(SDL_Window *window);
  ~SDLWindow();
  int Width() const;
  int Height() const;
  struct SDL_Window *Handle() const;
};

class SDLApp {
  class SDLAppImpl *impl_ = nullptr;

public:
  SDLApp(const SDLApp &) = delete;
  SDLApp &operator=(const SDLApp &) = delete;
  SDLApp();
  ~SDLApp();
  struct std::shared_ptr<SDLWindow> CreateWindow(int width, int height,
                                                 const char *title);
  bool NewFrame();
  std::span<char> DequeueInput();
};

} // namespace termtk
