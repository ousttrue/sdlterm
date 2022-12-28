fond_inc = include_directories('src')
fond = static_library('fond', [
    'src/fond.c',
    'src/fond_common.c',
    'src/fond_buffer.c',
    'src/fond_windows.c',
],
include_directories: [fond_inc])

fond_lib = fond
fond_dep = declare_dependency(link_with : fond_lib,
  include_directories : fond_inc)
