cmake_minimum_required(VERSION 3.22.1)

include(FetchContent)
FetchContent_Declare(
  sdl
  GIT_REPOSITORY https://github.com/libsdl-org/SDL
  GIT_TAG release-2.26.1
  GIT_PROGRESS TRUE)

# FetchContent_Declare( freetype2 GIT_REPOSITORY
# https://github.com/aseprite/freetype2.git GIT_TAG VER-2-10-0 GIT_PROGRESS
# TRUE)

FetchContent_MakeAvailable(sdl)

project(sdlterm)

subdirs(SDL_fox)

add_executable(${PROJECT_NAME} src/main.cpp src/sdlterm.cpp src/vtermapp.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL_fox vterm)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
