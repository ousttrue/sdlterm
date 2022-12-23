set(TARGET_NAME vterm)
add_library(
  ${TARGET_NAME}
  src/encoding.c
  src/keyboard.c
  src/mouse.c
  src/parser.c
  src/pen.c
  src/screen.c
  src/state.c
  src/unicode.c
  src/vterm.c)
target_include_directories(${TARGET_NAME} PUBLIC include)
