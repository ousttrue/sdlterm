#include "childprocess.h"
#include <iostream>
#include <pty.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

ChildProcess::ChildProcess() {}
ChildProcess::~ChildProcess() {
  std::cout << "Process exit status: " << status_ << std::endl;
}
void ChildProcess::createSubprocessWithPty(int rows, int cols, const char *prog,
                                           const std::vector<std::string> &args,
                                           const char *TERM) {
  int fd;
  struct winsize win = {(unsigned short)rows, (unsigned short)cols, 0, 0};
  auto pid = forkpty(&fd, NULL, NULL, &win);
  if (pid < 0)
    throw std::runtime_error("forkpty failed");
  // else
  if (!pid) {
    // child
    setenv("TERM", TERM, 1);
    char **argv = new char *[args.size() + 2];
    argv[0] = strdup(prog);
    for (int i = 1; i <= args.size(); i++) {
      argv[i] = strdup(args[i - 1].c_str());
    }
    argv[args.size() + 1] = NULL;
    if (execvp(prog, argv) < 0)
      exit(-1);
  }
  // else
  child_ = pid;
  pty_fd_ = fd;
}

bool ChildProcess::is_end() {
  auto done_pid = waitpid(child_, &status_, WNOHANG);
  return child_ == done_pid;
}

void ChildProcess::kill() {
  if (::kill(child_, SIGKILL) != 0) {
    std::cout << "fail to kill: " << child_ << " => " << errno << std::endl;
  }
}

void ChildProcess::write(const char *s, size_t len) {
  ::write(pty_fd_, s, len);
}

std::span<char> ChildProcess::processInput() {
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(pty_fd_, &readfds);

  timeval timeout = {0, 0};
  if (select(pty_fd_ + 1, &readfds, NULL, NULL, &timeout) > 0) {
    auto size = read(pty_fd_, buf_, sizeof(buf_));
    if (size > 0) {
      // TODO: write to vterm
      // input_write(buf, size);
      return {buf_, buf_ + size};
    }
  }
  return {};
}
