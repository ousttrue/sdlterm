include(FetchContent)

#
# GLFW
#
set(BUILD_SHARED_LIBS ON)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.8)
FetchContent_MakeAvailable(glfw) # 👈 FetchContent_Declare で指定した名前。target
                                 # ではない。多分、違う名前でも動く
#
# glew
#
FetchContent_Populate(
  glew
  URL https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0.tgz/download
)
add_library(glew SHARED ${glew_SOURCE_DIR}/src/glew.c
                        ${glew_SOURCE_DIR}/src/glewinfo.c)
target_include_directories(glew PUBLIC ${glew_SOURCE_DIR}/include)
target_compile_definitions(glew PRIVATE GLEW_BUILD # PUBLIC GLEW_STATIC
)
target_link_libraries(glew PUBLIC OpenGL32)

set(TARGET_NAME fond_test)
add_executable(${TARGET_NAME} main.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE fond glew glfw)
