#include "childprocess.h"
#include <iostream>
#include <pty.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

ChildProcess::~ChildProcess() {
  std::cout << "ChildProcess::~ChildProcess\n";

  kill(this->child_, SIGKILL);
  pid_t wpid;
  int wstatus;
  do {
    wpid = waitpid(this->child_, &wstatus, WUNTRACED | WCONTINUED);
    if (wpid == -1)
      break;
  } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
  this->child_ = wpid;
}

void ChildProcess::Write(const char *buf, size_t size) {
  write(this->childfd_, buf, size);
}

void ChildProcess::NotifyTermSize(uint16_t rows, uint16_t cols) {
  struct winsize ws = {
      .ws_row = rows,
      .ws_col = cols,
  };
  ioctl(this->childfd_, TIOCSWINSZ, &ws);
}

void ChildProcess::HandleOutputs() {
  fd_set rfds;
  struct timeval tv = {0};

  FD_ZERO(&rfds);
  FD_SET(this->childfd_, &rfds);

  tv.tv_sec = 0;
  tv.tv_usec = 50000;

  if (select(this->childfd_ + 1, &rfds, NULL, NULL, &tv) > 0) {
    char line[256];
    int n;
    if ((n = read(this->childfd_, line, sizeof(line))) > 0) {
      this->OutputCallback(line, n);
      // vterm_screen_flush_damage(this->screen);
    }
  }
}

static int childState = 0;

static void OnStopChild(int signum) { childState = 0; }

bool ChildProcess::Launch(const char *exec, char *const argv[]) {
  this->child_ = forkpty(&this->childfd_, NULL, NULL, NULL);
  if (this->child_ < 0) {
    return false;
  } else if (this->child_ == 0) {
    // child
    execvp(exec, argv);
    exit(0);
  } else {
    // parent
    struct sigaction action = {0};
    action.sa_handler = OnStopChild;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGCHLD, &action, NULL);
    childState = 1;
    return true;
  }
}

bool ChildProcess::Closed()const
{
  return childState == 0;
}
