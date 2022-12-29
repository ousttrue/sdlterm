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
struct fond_buffer {
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

  // Free all the data that was allocated
  // into the struct by fond_load_buffer. This
  // will /not/ free the font struct.
  void fond_free_buffer();

  // Load the buffer struct and allocate the
  // necessary OpenGL data.
  // The following fields must be set in the
  // struct:
  //   font
  //   width
  //   height
  int fond_load_buffer(const char *vs, const char *fs);

  // Render the given text to the buffer's
  // texture. The text will be rendered at the
  // given offset, with x and y being in pixels.
  // color can be either 0 for white text, or
  // an array of four floats, representing RGBA
  // of the text's colour in that order.
  int fond_render(const char *text, float x, float y, float *color);

  // Same as fond_render, but taking an UTF32
  // encoded string of codepoints and its size.
  int fond_render_u(int32_t *text, size_t size, float x, float y, float *color);
};
