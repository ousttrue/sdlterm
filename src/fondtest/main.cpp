#include "quad.h"
#include "text.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fond.h>
#include <fstream>
#include <stdint.h>
#include <stdio.h>
#include <vector>

struct FontAtlas {
  struct fond_font font = {0};
  struct fond_extent extent = {0};
  FontAtlas() {}
  ~FontAtlas() { fond_free(&font); }

  const struct fond_extent *Load(const char *file) {
    font = {
        .file = file,
        .size = 100.0,
        .characters = "abcdefghijklmnopqrstuvwxyz"
                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "0123456789 \n\t.,-;:?!/()*+^_\\\"'",
        .oversample = 2,
    };
    printf("Loading font... ");
    GLint max_size = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);
    if (!fond_load_fit(&font, max_size)) {
      printf("Error: %s\n", fond_error_string(fond_error()));
      return nullptr;
    }
    printf("DONE (Atlas %ix%i)\n", font.width, font.height);

    fond_compute_extent_u(&font, 0, 0, &extent);
    return &extent;
  }
};

void character_callback(GLFWwindow *window, unsigned int codepoint) {
  auto data = (TextTexture *)glfwGetWindowUserPointer(window);
  data->push(codepoint);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mode) {
  auto data = (TextTexture *)glfwGetWindowUserPointer(window);
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

  Quad quad;
  if (!quad.Load()) {
    printf("FAILED\n");
    return 6;
  }
  printf("DONE\n");

  FontAtlas font;
  auto extent = font.Load(argv[1]);
  if (!extent) {
    return 7;
  }

  TextTexture text(&font.font, *extent);
  if (!text.Load(vs.data(), fs.data())) {
    return 8;
  }
  glfwSetWindowUserPointer(window, &text);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // clear
    glViewport(0, 0, width, height);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    text.Bind();
    quad.Render();

    glfwSwapBuffers(window);
  }

  return 0;
}
