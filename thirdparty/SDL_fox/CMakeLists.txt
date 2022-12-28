# https://github.com/palomena/SDL_fox
set(TARGET_NAME SDL_fox)
add_library(${TARGET_NAME} STATIC SDL_fox.c)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}
                                                 ${sdl_SOURCE_DIR}/include)
target_link_libraries(${TARGET_NAME} PUBLIC freetype SDL2)
# target_compile_definitions(${TARGET_NAME} PUBLIC DLL_EXPORT)
