set(TARGET_NAME termtk)
add_library(${TARGET_NAME} STATIC sdl_app.cpp vterm_object.cpp)
if(WIN32)
  target_sources(${TARGET_NAME} PRIVATE childprocess_windows.cpp)
else()
  target_sources(${TARGET_NAME} PRIVATE childprocess_unix.cpp)
endif()
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
  ${TARGET_NAME}
  PRIVATE SDL2 SDL2main SDL_fox
  PUBLIC vterm)
target_compile_definitions(${TARGET_NAME} PRIVATE NOMINMAX)
