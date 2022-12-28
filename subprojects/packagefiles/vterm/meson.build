project('vterm', 'c')
vterm_inc = include_directories('src', '.', 'include')
vterm = static_library('vterm', [
  'src/encoding.c',
  'src/keyboard.c',
  'src/mouse.c',
  'src/parser.c',
  'src/pen.c',
  'src/screen.c',
  'src/state.c',
  'src/unicode.c',
  'src/vterm.c'],
include_directories: vterm_inc)

vterm_lib = vterm
vterm_dep = declare_dependency(link_with : vterm_lib,
  include_directories : vterm_inc)
