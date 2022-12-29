#include "text.h"
#include <GL/glew.h>
#include <stdio.h>

TextTexture::TextTexture(struct fond_font *font, const fond_extent &extent) : y(extent.t) {
  buffer.font = font;
  fond_free_buffer(&buffer);
}

bool TextTexture::Load(const char *vs, const char *fs) {
  buffer.width = 800;
  buffer.height = 600;
  printf("Loading buffer... ");
  if (!fond_load_buffer(&buffer, vs, fs)) {
    printf("Error: %s\n", fond_error_string(fond_error()));
    return false;
  }
  printf("DONE\n");

  printf("Rendering buffer... ");
  if (!fond_render(&buffer, "Type it", 0, 100, 0)) {
    printf("Error: %s\n", fond_error_string(fond_error()));
    return false;
  }
  printf("DONE\n");

  return true;
}

void TextTexture::Bind() { glBindTexture(GL_TEXTURE_2D, buffer.texture); }

bool TextTexture::render_text() {
  if (!fond_render_u(&buffer, text, pos, 0, y, 0)) {
    printf("Failed to render: %s\n", fond_error_string(fond_error()));
    return 0;
  }
  return 1;
}

void TextTexture::push(uint32_t codepoint) {
  if (pos < 500) {
    text[pos] = (int32_t)codepoint;
    pos++;
    if (!render_text())
      pos--;
  }
}

void TextTexture::pop() {
  if (0 < pos) {
    pos--;
    if (!render_text())
      pos++;
  }
}
