include(FetchContent)
FetchContent_Declare(
  sdl
  GIT_REPOSITORY https://github.com/libsdl-org/SDL
  GIT_TAG release-2.0.22
  GIT_PROGRESS TRUE)

FetchContent_Declare(
  freetype2
  GIT_REPOSITORY https://github.com/aseprite/freetype2.git
  GIT_TAG VER-2-10-0
  GIT_PROGRESS TRUE)

FetchContent_Declare(
  sdl_ttf
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf
  GIT_TAG release-2.20.1
  GIT_PROGRESS TRUE)

FetchContent_MakeAvailable(sdl freetype2 sdl_ttf)

FetchContent_Populate(
  vterm URL https://www.leonerd.org.uk/code/libvterm/libvterm-0.3.tar.gz)

add_library(
  vterm
  ${vterm_SOURCE_DIR}/src/encoding.c
  ${vterm_SOURCE_DIR}/src/keyboard.c
  ${vterm_SOURCE_DIR}/src/mouse.c
  ${vterm_SOURCE_DIR}/src/parser.c
  ${vterm_SOURCE_DIR}/src/pen.c
  ${vterm_SOURCE_DIR}/src/screen.c
  ${vterm_SOURCE_DIR}/src/state.c
  ${vterm_SOURCE_DIR}/src/unicode.c
  ${vterm_SOURCE_DIR}/src/vterm.c)
target_include_directories(vterm PUBLIC ${vterm_SOURCE_DIR}/include
                                        ${CMAKE_CURRENT_LIST_DIR}/vterm_src)

subdirs(SDL_fox)

if(WIN32)
  FetchContent_Populate(
    getopt
    GIT_REPOSITORY https://github.com/takamin/win-c.git
    GIT_TAG v1.0)
  add_library(getopt ${getopt_SOURCE_DIR}/source/getopt.c)
  target_include_directories(getopt PUBLIC ${getopt_SOURCE_DIR}/include)
endif()
