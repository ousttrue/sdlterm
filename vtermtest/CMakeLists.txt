set(TARGET_NAME vtermtest)
add_executable(${TARGET_NAME} main.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE SDL2 SDL2main SDL2_ttf
                                             freetype termtk)
