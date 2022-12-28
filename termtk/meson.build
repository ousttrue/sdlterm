termtk = static_library('termtk', [
    'childprocess_windows.cpp', 
    'sdl_app.cpp', 
    'vterm_object.cpp'
    ],
    dependencies: [sdl2_dep, vterm_dep])

termtk_inc = include_directories('.')
termtk_lib = termtk
termtk_dep = declare_dependency(link_with : termtk_lib,
  include_directories : termtk_inc)
