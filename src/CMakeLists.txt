set(TARGET_NAME sdlterm)
add_executable(${TARGET_NAME} main.cpp sdlterm.cpp sdlrenderer.cpp vtermapp.cpp
                              term_config.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main SDL_fox
                                             vterm)
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)
target_compile_definitions(${TARGET_NAME} PRIVATE NOMINMAX)

if(WIN32)
  target_link_libraries(${TARGET_NAME} PRIVATE getopt)
  target_compile_definitions(${TARGET_NAME} PRIVATE NOMINMAX)
  target_sources(${TARGET_NAME} PRIVATE childprocess_windows.cpp)
else()
  target_sources(${TARGET_NAME} PRIVATE childprocess_unix.cpp)
endif()
