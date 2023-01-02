#include "text.h"
#include <GL/glew.h>
#include <fond.h>
#include <stdio.h>

TextTexture::TextTexture(struct fond_font *font, const fond_extent &extent)
    : buffer(font), y(extent.t) {}

bool TextTexture::Initialize(int width, int height) {
  printf("Loading buffer... ");
  if (!buffer.initialize(width, height)) {
    printf("Error: %s\n", fond_error_string(fond_error()));
    return false;
  }
  printf("DONE\n");

  printf("Rendering buffer... ");
  if (!buffer.render("Type it", 0, 100, 0)) {
    printf("Error: %s\n", fond_error_string(fond_error()));
    return false;
  }
  printf("DONE\n");

  return true;
}

bool TextTexture::render_text() {
  if (!buffer.render_u(text, pos, 0, y, 0)) {
    printf("Failed to render: %s\n", fond_error_string(fond_error()));
    return 0;
  }
  return 1;
}

void TextTexture::Push(uint32_t codepoint) {
  if (pos < 500) {
    text[pos] = (int32_t)codepoint;
    pos++;
    if (!render_text())
      pos--;
  }
}

void TextTexture::Pop() {
  if (0 < pos) {
    pos--;
    if (!render_text())
      pos++;
  }
}
