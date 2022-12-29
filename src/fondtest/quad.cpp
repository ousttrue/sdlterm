#include "quad.h"

const GLchar *vertex_shader = R"(#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 in_texcoord;
out vec2 texcoord;

void main(){
  gl_Position = vec4(position, 1.0);
  texcoord = in_texcoord;
}
)";

const GLchar *fragment_shader = R"(
#version 330 core
uniform sampler2D tex_image;
in vec2 texcoord;
out vec4 color;

void main(){
  color = texture(tex_image, texcoord);
}
)";

const GLfloat vertices[] = {1.0f, 1.0f,  0.0f, 1.0f,  1.0f,  1.0f, -1.0f,
                            0.0f, 1.0f,  0.0f, -1.0f, -1.0f, 0.0f, 0.0f,
                            0.0f, -1.0f, 1.0f, 0.0f,  0.0f,  1.0f};
const GLuint indices[] = {0, 3, 1, 1, 3, 2};

Quad::Quad() {
  // glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  vert = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert, 1, &vertex_shader, 0);
  glCompileShader(vert);

  frag = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag, 1, &fragment_shader, 0);
  glCompileShader(frag);

  program = glCreateProgram();
  glAttachShader(program, vert);
  glAttachShader(program, frag);
  glLinkProgram(program);
}

bool Quad::Load() {
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (GLvoid *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (GLvoid *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  if (glGetError() != GL_NO_ERROR) {
    return false;
  }
  return true;
}

void Quad::Render() {
  glUseProgram(program);
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
