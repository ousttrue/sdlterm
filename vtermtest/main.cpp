#include "vtermtest.h"
#include <iostream>
#include <signal.h>
#include <sdl_app.h>

auto FONT_FILE = "/usr/share/fonts/vlgothic/VL-Gothic-Regular.ttf";

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
  // TTF_Font* font = TTF_OpenFont("RictyDiminished-Regular.ttf", 48);
  if (font == NULL) {
    std::cerr << "TTF_OpenFont: " << TTF_GetError() << std::endl;
    return 3;
  }

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
  auto subprocess = createSubprocessWithPty(rows, cols, getenv("SHELL"), {"-"});

  Terminal terminal(subprocess.second /*fd*/, rows, cols, font);

  auto pid = subprocess.first;
  std::pair<pid_t, int> rst;
  while ((rst = waitpid(pid, WNOHANG)).first != pid) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_QUIT ||
          (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE &&
           (ev.key.keysym.mod & KMOD_CTRL))) {
        kill(pid, SIGTERM);
      } else {
        terminal.processEvent(ev);
      }
    }

    terminal.processInput();

    SDL_Rect rect = {0, 0, 1024, 768};
    terminal.render(renderer, rect);
    SDL_RenderPresent(renderer);
  }
  std::cout << "Process exit status: " << rst.second << std::endl;

  TTF_Quit();
  return 0;
}
