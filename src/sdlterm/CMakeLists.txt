set(TARGET_NAME sdlterm)
add_executable(${TARGET_NAME} main.cpp sdlrenderer.cpp term_config.cpp)
target_link_libraries(
  ${TARGET_NAME}
  PRIVATE SDL2
          SDL2main
          SDL_fox
          vterm
          termtk
          winmm
          user32
          version
          imm32
          Setupapi)
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)
target_compile_definitions(${TARGET_NAME} PRIVATE NOMINMAX)

if(WIN32)
  target_link_libraries(${TARGET_NAME} PRIVATE getopt)
  target_compile_definitions(${TARGET_NAME} PRIVATE NOMINMAX)
else()

endif()
