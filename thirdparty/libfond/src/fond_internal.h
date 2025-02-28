#include "fond.h"
#include <stdio.h>
#if defined(WIN32) || defined(_WIN32)
#define FOND_WIN
#  include <windows.h>
#  include <GL/gl.h>
#  include <GL/glext.h>
#  include "fond_windows.h"
#endif
#if defined(__APPLE__)
#  define FOND_MAC
#  define GL_GLEXT_PROTOTYPES
#  include <OpenGL/gl3.h>
#  include <OpenGL/gl3ext.h>
#endif
#if defined(__linux__)
#  define FOND_LIN
#  define GL_GLEXT_PROTOTYPES
#  include <GL/gl.h>
#  include <GL/glext.h>
#endif

extern int errorcode;

int fond_load_file(char *file, void **content);
