set(TARGET_NAME vtermtest)
add_executable(${TARGET_NAME} vtermtest.cpp)
target_link_libraries(
  ${TARGET_NAME}
  PRIVATE SDL2::SDL2
          SDL2::SDL2main
          SDL2_ttf::SDL2_ttf
          icuio
          icui18n
          icuuc
          icudata
          vterm)
