#pragma once
#include "fond_buffer.h"

class TextTexture {
  fond_buffer buffer;
  int32_t text[500];
  size_t pos = 0;
  float y = 0;

public:
  TextTexture(struct fond_font *font, const struct fond_extent &extent);
  bool Initialize(int width, int height);
  void Bind() { buffer.bind(); }
  void Push(uint32_t codepoint);
  void Pop();

private:
  bool render_text();
};
