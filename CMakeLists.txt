cmake_minimum_required(VERSION 3.22.1)
project(sdlterm)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

subdirs(thirdparty src vtermtest)
