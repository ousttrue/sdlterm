#pragma once
#include "fond.h"

class TextTexture {
  struct fond_buffer buffer = {0};
  int32_t text[500];
  size_t pos;
  float y = 0;

public:
  TextTexture(struct fond_font *font, const fond_extent &extent);
  bool Load(const char *vs, const char *fs);
  void Bind();
  void push(uint32_t codepoint);
  void pop();

private:
  bool render_text();
};
