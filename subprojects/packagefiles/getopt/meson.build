project('getopt', 'c')
getopt_inc = include_directories('include')
getopt = static_library('getopt', ['source/getopt.c'],
include_directories: getopt_inc)

getopt_lib = getopt
getopt_dep = declare_dependency(link_with : getopt_lib,
  include_directories : getopt_inc)
