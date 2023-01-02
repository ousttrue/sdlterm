#include "quad.h"
#include "text.h"
#include <GLFW/glfw3.h>
#include <fond.h>
#include <stdio.h>

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
  auto text = (TextTexture *)glfwGetWindowUserPointer(window);
  text->Push(codepoint);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mode) {
  auto text = (TextTexture *)glfwGetWindowUserPointer(window);
  if (action == GLFW_RELEASE) {
    switch (key) {
    case GLFW_KEY_BACKSPACE:
      text->Pop();
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
  if (argc < 2) {
    printf("Please specify a TTF file to load.\n");
    return 1;
  }

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
  if (!text.Initialize(800, 600)) {
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
