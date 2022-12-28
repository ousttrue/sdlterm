#pragma once
#include <span>
#include <string>
#include <sys/types.h>
#include <vector>

namespace termtk {

class ChildProcess {
  struct ChildProcessImpl *impl_ = nullptr;
  // pid_t child_pid_ = 0;
  // int pty_fd_ = 0;
  // int status_ = 0;
  char buf_[8192];

public:
  ChildProcess();
  ~ChildProcess();
  void Launch(int rows, int cols, const char *prog,
              const std::vector<std::string> &args = {},
              const char *TERM = "xterm-256color");
  bool IsClosed();
  void Kill();
  void NotifyTermSize(unsigned short rows, unsigned short cols);
  void Write(const char *s, size_t len);
  static void Write(const char *s, size_t len, void *user) {
    auto self = (ChildProcess *)user;
    self->Write(s, len);
  }
  std::span<char> Read();
};

} // namespace termtk
