#include "childprocess.h"
#include <iostream>
#include <pty.h>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>

// static int childState = 0;
// static void OnStopChild(int signum) { childState = 0; }

namespace termtk {

ChildProcess::ChildProcess() {}

ChildProcess::~ChildProcess() {
  std::cout << "Process exit status: " << status_ << std::endl;

  kill(child_pid_, SIGKILL);
  pid_t wpid;
  int wstatus;
  do {
    wpid = waitpid(child_pid_, &wstatus, WUNTRACED | WCONTINUED);
    if (wpid == -1)
      break;
  } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
  child_pid_ = wpid;
}

void ChildProcess::Launch(int rows, int cols, const char *prog,
                          const std::vector<std::string> &args,
                          const char *TERM) {
  struct winsize win = {(unsigned short)rows, (unsigned short)cols, 0, 0};
  child_pid_ = forkpty(&pty_fd_, NULL, NULL, &win);
  if (child_pid_ < 0) {
    throw std::runtime_error("forkpty failed");
  } else if (child_pid_ == 0) {
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
    // child
    // execvp(exec, argv);
    // exit(0);
  } else {
    // parent
    // struct sigaction action = {0};
    // action.sa_handler = OnStopChild;
    // action.sa_flags = 0;
    // sigemptyset(&action.sa_mask);
    // sigaction(SIGCHLD, &action, NULL);
    // childState = 1;
  }
}

// bool ChildProcess::Closed() const { return childState == 0; }
bool ChildProcess::IsClosed() {
  auto done_pid = ::waitpid(child_pid_, &status_, WNOHANG);
  return child_pid_ == done_pid;
}

void ChildProcess::Kill() {
  if (::kill(child_pid_, SIGKILL) != 0) {
    std::cout << "fail to kill: " << child_pid_ << " => " << errno << std::endl;
  }
}

void ChildProcess::Write(const char *buf, size_t size) {
  write(pty_fd_, buf, size);
}
// void ChildProcess::Write(const char *s, size_t len) {
//   ::write(pty_fd_, s, len);
// }

std::span<char> ChildProcess::Read() {
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(pty_fd_, &readfds);
  // fd_set rfds;
  // FD_ZERO(&rfds);
  // FD_SET(child_fd_, &rfds);
  timeval timeout = {0, 0};
  // struct timeval tv = {0};
  // tv.tv_sec = 0;
  // tv.tv_usec = 50000;
  if (select(pty_fd_ + 1, &readfds, NULL, NULL, &timeout) > 0) {
    auto size = ::read(pty_fd_, buf_, sizeof(buf_));
    if (size > 0) {
      // TODO: write to vterm
      // input_write(buf, size);
      return {buf_, buf_ + size};
    }
  }
  return {};
}

} // namespace termtk