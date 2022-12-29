#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fond.h>
#include <fstream>
#include <stdint.h>
#include <stdio.h>
#include <vector>

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

struct data {
  int32_t text[500];
  size_t pos;
};

class Font {
  struct fond_buffer buffer = {0};
  struct fond_font font = {0};
  data data = {};

public:
  Font() { buffer.font = &font; }
  ~Font() {
    fond_free(&font);
    fond_free_buffer(&buffer);
  }

  bool Load(const char *file, const char *vs, const char *fs) {
    font.file = file;
    font.size = 100.0;
    font.oversample = 2;
    font.characters = "abcdefghijklmnopqrstuvwxyz"
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "0123456789 \n\t.,-;:?!/()*+^_\\\"'";

    printf("Loading font... ");
    GLint max_size = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);
    if (!fond_load_fit(&font, max_size)) {
      printf("Error: %s\n", fond_error_string(fond_error()));
      return false;
    }
    printf("DONE (Atlas %ix%i)\n", font.width, font.height);

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

  void Bind() { glBindTexture(GL_TEXTURE_2D, buffer.texture); }

  bool render_text() {
    struct fond_extent extent = {0};
    fond_compute_extent_u(&font, 0, 0, &extent);
    if (!fond_render_u(&buffer, data.text, data.pos, 0, extent.t, 0)) {
      printf("Failed to render: %s\n", fond_error_string(fond_error()));
      return 0;
    }
    return 1;
  }

  void push(uint32_t codepoint) {
    if (data.pos < 500) {
      data.text[data.pos] = (int32_t)codepoint;
      data.pos++;
      if (!render_text())
        data.pos--;
    }
  }

  void pop() {
    if (0 < data.pos) {
      data.pos--;
      if (!render_text())
        data.pos++;
    }
  }
};

class Renderer {
  GLuint vert, frag, program, vbo, ebo, vao;

public:
  Font font;
  Renderer() {
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

  bool CreateVertexBuffer(const void *vertices, size_t vertices_size,
                          const void *indices, size_t indices_size) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices_size, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (GLvoid *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (GLvoid *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, indices,
                 GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if(glGetError() != GL_NO_ERROR)
    {
      return false;
    }
    return true;
  }

  bool LoadFont(const char *file, const char *vs, const char *fs) {
    return font.Load(file, vs, fs);
  }

  void Render(int width, int height) {
    glViewport(0, 0, width, height);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    font.Bind();
    glUseProgram(program);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
};

void character_callback(GLFWwindow *window, unsigned int codepoint) {
  auto data = (Font *)glfwGetWindowUserPointer(window);
  data->push(codepoint);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mode) {
  auto data = (Font *)glfwGetWindowUserPointer(window);
  if (action == GLFW_RELEASE) {
    switch (key) {
    case GLFW_KEY_BACKSPACE:
      data->pop();
      break;
    case GLFW_KEY_ENTER:
      character_callback(window, '\n');
      break;
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window, GL_TRUE);
      break;
    }
  }
}

static std::vector<char> ReadAllBytes(char const *filename) {
  std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
  auto pos = ifs.tellg();
  std::vector<char> buffer(pos);
  ifs.seekg(0, std::ios::beg);
  ifs.read(buffer.data(), pos);
  return buffer;
}

class App {
  GLFWwindow *window_ = nullptr;

public:
  App() { glfwInit(); }
  ~App() { glfwTerminate(); }
  GLFWwindow *CreateWindow(int width, int height, const char *title) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window_ = glfwCreateWindow(width, height, title, 0, 0);
    if (window_ == 0) {
      printf("Failed to create GLFW window\n");
      return nullptr;
    }
    glfwMakeContextCurrent(window_);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
      printf("Failed to initialize GLEW\n");
      return nullptr;
    }

    printf("%s\n", glGetString(GL_VERSION));
    printf("%s\n", glGetString(GL_VENDOR));
    printf("%s\n", glGetString(GL_RENDERER));

    return window_;
  }
};

int main(int argc, char **argv) {
  if (argc < 4) {
    printf("Please specify a TTF file to load.\n");
    return 1;
  }
  auto vs = ReadAllBytes(argv[2]);
  if (vs.empty()) {
    return 2;
  }
  vs.push_back(0);
  auto fs = ReadAllBytes(argv[3]);
  if (fs.empty()) {
    return 3;
  }
  fs.push_back(0);

  printf("Initializing GL... ");
  App app;
  auto window = app.CreateWindow(800, 600, "Fond Test");
  if (!window) {
    return 4;
  }

  glfwSetKeyCallback(window, key_callback);
  glfwSetCharCallback(window, character_callback);

  Renderer renderer;
  glfwSetWindowUserPointer(window, &renderer.font);

  if (!renderer.LoadFont(argv[1], vs.data(), fs.data())) {
    return 5;
  }

  GLfloat vertices[] = {1.0f, 1.0f,  0.0f, 1.0f,  1.0f,  1.0f, -1.0f,
                        0.0f, 1.0f,  0.0f, -1.0f, -1.0f, 0.0f, 0.0f,
                        0.0f, -1.0f, 1.0f, 0.0f,  0.0f,  1.0f};
  GLuint indices[] = {0, 3, 1, 1, 3, 2};

  if (!renderer.CreateVertexBuffer(vertices, sizeof(vertices), indices,
                                   sizeof(indices))) {
    printf("FAILED\n");
    return 6;
  }
  printf("DONE\n");

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    renderer.Render(width, height);

    glfwSwapBuffers(window);
  }

  return 0;
}
