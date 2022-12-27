#pragma once
#include <span>
#include <string>
#include <sys/types.h>
#include <vector>

class ChildProcess {
  pid_t child_ = 0;
  // child process stdout
  int pty_fd_ = 0;
  int status_ = 0;
  char buf_[4096];

public:
  ChildProcess();
  ~ChildProcess();
  void createSubprocessWithPty(int rows, int cols, const char *prog,
                               const std::vector<std::string> &args = {},
                               const char *TERM = "xterm-256color");
  bool is_end();
  void kill();
  void write(const char *s, size_t len);
  static void output_callback(const char *s, size_t len, void *user) {
    auto self = (ChildProcess *)user;
    self->write(s, len);
  }
  std::span<char> processInput();
};
