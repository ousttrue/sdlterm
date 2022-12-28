sdl2_fox_inc = include_directories('.')
sdl2_fox = static_library('SDL_fox', ['SDL_fox.c'],
dependencies: [sdl2_dep, freetype_dep])

sdl2_fox_lib = sdl2_fox
sdl2_fox_dep = declare_dependency(link_with : sdl2_fox_lib,
  include_directories : sdl2_fox_inc)
