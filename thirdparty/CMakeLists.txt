include(FetchContent)

# Set defaults preventing destination file conflicts
set(SDL_CMAKE_DEBUG_POSTFIX "")
FetchContent_Populate(
  sdl
  GIT_REPOSITORY https://github.com/libsdl-org/SDL
  GIT_TAG release-2.0.22
  GIT_PROGRESS TRUE)

# FetchContent_Declare( freetype2 GIT_REPOSITORY
# https://github.com/aseprite/freetype2.git GIT_TAG VER-2-10-0 GIT_PROGRESS
# TRUE)

set(SDL2TTF_VENDORED ON)

FetchContent_Populate(
  sdl_ttf
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf
  GIT_TAG release-2.20.1
  GIT_PROGRESS TRUE)

set(BUILD_SHARED_LIBS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
FetchContent_Populate(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.8)
# FetchContent_MakeAvailable(sdl sdl_ttf glfw)
subdirs(${sdl_SOURCE_DIR} ${sdl_ttf_SOURCE_DIR} ${glfw_SOURCE_DIR})

FetchContent_Populate(
  vterm URL https://www.leonerd.org.uk/code/libvterm/libvterm-0.3.tar.gz)

add_library(
  vterm STATIC
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


subdirs(SDL_fox libfond)

if(WIN32)
  FetchContent_Populate(
    getopt
    GIT_REPOSITORY https://github.com/takamin/win-c.git
    GIT_TAG v1.0)
  add_library(getopt STATIC ${getopt_SOURCE_DIR}/source/getopt.c)
  target_include_directories(getopt PUBLIC ${getopt_SOURCE_DIR}/include)
endif()
