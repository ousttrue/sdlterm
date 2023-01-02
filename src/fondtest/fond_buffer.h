#pragma once
#include <stdint.h>

// This struct allows for convenience in
// rendering, as it will render text for you
// into a texture, which you can then render
// like any other. Thus you won't need to
// handle the actual rendering logic yourself.
//
// See fond_free_buffer
// See fond_load_buffer
// See fond_render
// See fond_render_u
class fond_buffer {
  // Pointer to the font that it renders.
  struct fond_font *font = 0;
  // The OpenGL texture ID to which this
  // buffer renders to.
  unsigned int texture = 0;
  // The width of the texture.
  unsigned int width = 0;
  // The height of the texture.
  unsigned int height = 0;
  // Internal data.
  unsigned int program = 0;
  unsigned int framebuffer = 0;
  unsigned int vao = 0;
  unsigned int vbo = 0;
  unsigned int ebo = 0;

public:
  fond_buffer(const fond_buffer &) = delete;
  fond_buffer &operator=(const fond_buffer &) = delete;

  fond_buffer(struct fond_font *font);

  // Free all the data that was allocated
  // into the struct by fond_load_buffer. This
  // will /not/ free the font struct.
  ~fond_buffer();

  // Load the buffer struct and allocate the
  // necessary OpenGL data.
  bool initialize(int width, int height);

  // Render the given text to the buffer's
  // texture. The text will be rendered at the
  // given offset, with x and y being in pixels.
  // color can be either 0 for white text, or
  // an array of four floats, representing RGBA
  // of the text's colour in that order.
  bool render(const char *text, float x, float y, float *color);

  // Same as fond_render, but taking an UTF32
  // encoded string of codepoints and its size.
  bool render_u(int32_t *text, size_t size, float x, float y, float *color);

  void bind();
};
