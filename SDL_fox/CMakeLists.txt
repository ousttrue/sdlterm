set(TARGET_NAME SDL_fox)
add_library(${TARGET_NAME} SDL_fox.c)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}
                                                 /usr/include/freetype2)
target_link_libraries(${TARGET_NAME} PUBLIC freetype SDL2::SDL2)
