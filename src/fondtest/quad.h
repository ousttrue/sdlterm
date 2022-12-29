#pragma once
#include <GL/glew.h>

class Quad {
  GLuint vert, frag, program, vbo, ebo, vao;

public:
  Quad();
  bool Load();
  void Render();
};
