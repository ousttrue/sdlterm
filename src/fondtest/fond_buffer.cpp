#include "fond_buffer.h"
#include <GL/glew.h>
#include <fond.h>
#include <stdio.h>

const GLchar *to_texture_vert_src = R"(#version 330 core
uniform vec4 extent = vec4(1.0);

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 in_tex_coord;
out vec2 tex_coord;

void main(){
  gl_Position = vec4(2*(position.x+extent.x)/extent.z-1.0,
                     2*(position.y-extent.y)/extent.w+1.0,
                     0.0, 1.0);
  tex_coord = in_tex_coord;
}  
)";

const GLchar *to_texture_frag_src = R"(#version 330 core

uniform sampler2D tex_image;
uniform vec4 text_color = vec4(1.0, 1.0, 1.0, 1.0);

in vec2 tex_coord;
out vec4 color;

void main(){
  float intensity = texture(tex_image, tex_coord).r;
  color = vec4(intensity)*text_color;
}
)";

static int fond_check_shader(GLuint shader) {
  GLint res = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
  if (res == GL_FALSE) {
    GLint length = 0;
    GLchar error[GL_INFO_LOG_LENGTH];
    glGetShaderInfoLog(shader, length, &length, &error[0]);
    fprintf(stderr, "\nFond: GLSL error: %s\n", error);
    return 0;
  }
  return 1;
}

static int fond_check_program(GLuint program) {
  GLint res = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &res);
  if (res == GL_FALSE) {
    GLint length = 0;
    GLchar error[GL_INFO_LOG_LENGTH];
    glGetProgramInfoLog(program, length, &length, &error[0]);
    fprintf(stderr, "\nFond: GLSL error: %s\n", error);
    return 0;
  }
  return 1;
}

fond_buffer::fond_buffer(struct fond_font *font) : font(font) {}

fond_buffer::~fond_buffer() {
  if (this->texture)
    glDeleteTextures(1, &this->texture);
  this->texture = 0;

  if (this->program)
    glDeleteProgram(this->program);
  this->program = 0;

  if (this->framebuffer)
    glDeleteFramebuffers(1, &this->framebuffer);
  this->framebuffer = 0;
}

bool fond_buffer::initialize(int width, int height) {
  this->width = width;
  this->height = height;

  glGenTextures(1, &this->texture);
  glBindTexture(GL_TEXTURE_2D, this->texture);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.65f);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
  if (!fond_check_glerror()) {
    fond_err(FOND_OPENGL_ERROR);
    return false;
  }

  glGenFramebuffers(1, &this->framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         this->texture, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  if (!fond_check_glerror() ||
      glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    fond_err(FOND_OPENGL_ERROR);
    return false;
  }

  auto vert = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert, 1, &to_texture_vert_src, 0);
  glCompileShader(vert);
  if (!fond_check_shader(vert)) {
    fond_err(FOND_OPENGL_ERROR);
    return false;
  }

  auto frag = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag, 1, &to_texture_frag_src, 0);
  glCompileShader(frag);
  if (!fond_check_shader(frag)) {
    fond_err(FOND_OPENGL_ERROR);
    return false;
  }

  this->program = glCreateProgram();
  glAttachShader(this->program, vert);
  glAttachShader(this->program, frag);
  glLinkProgram(this->program);
  if (!fond_check_program(this->program)) {
    fond_err(FOND_OPENGL_ERROR);
    return false;
  }

  glGenVertexArrays(1, &this->vao);
  glGenBuffers(1, &this->vbo);
  glGenBuffers(1, &this->ebo);

  glBindVertexArray(this->vao);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4,
                        (GLvoid *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4,
                        (GLvoid *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  if (!fond_check_glerror()) {
    fond_err(FOND_OPENGL_ERROR);
    return false;
  }

  fond_err(FOND_NO_ERROR);
  glDeleteShader(vert);
  glDeleteShader(frag);
  return true;
}

// fond_load_buffer_cleanup:
//   if (this->texture)
//     glDeleteTextures(1, &this->texture);
//   this->texture = 0;

//   if (this->framebuffer)
//     glDeleteFramebuffers(1, &this->framebuffer);
//   this->framebuffer = 0;

//   if (vert)
//     glDeleteShader(vert);

//   if (frag)
//     glDeleteShader(frag);

//   if (this->program)
//     glDeleteProgram(this->program);
//   this->program = 0;

//   if (this->ebo)
//     glDeleteBuffers(1, &this->ebo);

//   if (this->vbo)
//     glDeleteBuffers(1, &this->vbo);

//   if (this->vao)
//     glDeleteVertexArrays(1, &this->vao);
//   return 0;
// }

bool fond_buffer::render(const char *text, float x, float y, float *color) {
  size_t size = 0;
  int32_t *codepoints = 0;

  if (!fond_decode_utf8((void *)text, &codepoints, &size)) {
    return false;
  }

  render_u(codepoints, size, x, y, color);
  free(codepoints);
  // return (errorcode == FOND_NO_ERROR);
  return true;
}

bool fond_buffer::render_u(int32_t *text, size_t size, float x, float y,
                               float *color) {
  size_t n;
  GLuint extent_u = 0, color_u = 0;

  if (!fond_update_u(this->font, text, size, &n, this->vbo, this->ebo)) {
    return 0;
  }

  extent_u = glGetUniformLocation(this->program, "extent");
  color_u = glGetUniformLocation(this->program, "text_color");

  glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);
  glViewport(0, 0, this->width, this->height);
  glUseProgram(this->program);
  glBindVertexArray(this->vao);
  glBindTexture(GL_TEXTURE_2D, this->font->atlas);
  glUniform4f(extent_u, x, y, this->width, this->height);
  if (color)
    glUniform4f(color_u, color[0], color[1], color[2], color[3]);
  {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, n, GL_UNSIGNED_INT, 0);
  }
  glBindTexture(GL_TEXTURE_2D, this->texture);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  if (!fond_check_glerror()) {
    fond_err(FOND_OPENGL_ERROR);
    return 0;
  }

  fond_err(FOND_NO_ERROR);
  return 1;
}

void fond_buffer::bind() { glBindTexture(GL_TEXTURE_2D, texture); }
