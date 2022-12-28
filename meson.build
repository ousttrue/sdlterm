project('sdlterm', ['c', 'cpp'],
    default_options: ['cpp_std=c++20'])

sdl2_dep = dependency('sdl2')
sdl2_ttf_dep = dependency('sdl2_ttf')
vterm_dep = dependency('vterm')
glew_dep = dependency('glew')
glfw_dep = dependency('glfw3')
freetype_dep = dependency('freetype')
getopt_dep = dependency('getopt')

subdir('thirdparty')
subdir('termtk')
subdir('src')
