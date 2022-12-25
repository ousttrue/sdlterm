#include "childprocess.h"
#include <iostream>
#include <pty.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

static pid_t child_ = 0;
static int childfd_ = 0;

ChildProcess::~ChildProcess() {
  std::cout << "ChildProcess::~ChildProcess\n";

  kill(child_, SIGKILL);
  pid_t wpid;
  int wstatus;
  do {
    wpid = waitpid(child_, &wstatus, WUNTRACED | WCONTINUED);
    if (wpid == -1)
      break;
  } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
  child_ = wpid;
}

void ChildProcess::Write(const char *buf, size_t size) {
  write(childfd_, buf, size);
}

void ChildProcess::NotifyTermSize(uint16_t rows, uint16_t cols) {
  struct winsize ws = {
      .ws_row = rows,
      .ws_col = cols,
  };
  ioctl(childfd_, TIOCSWINSZ, &ws);
}

void ChildProcess::HandleOutputs() {
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(childfd_, &rfds);

  struct timeval tv = {0};
  tv.tv_sec = 0;
  tv.tv_usec = 50000;

  if (select(childfd_ + 1, &rfds, NULL, NULL, &tv) > 0) {
    char line[256];
    int n;
    if ((n = read(childfd_, line, sizeof(line))) > 0) {
      this->OutputCallback(line, n);
      // vterm_screen_flush_damage(this->screen);
    }
  }
}

static int childState = 0;

static void OnStopChild(int signum) { childState = 0; }

bool ChildProcess::Launch(const char *exec, char *const argv[]) {
  child_ = forkpty(&childfd_, NULL, NULL, NULL);
  if (child_ < 0) {
    return false;
  } else if (child_ == 0) {
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

bool ChildProcess::Closed() const { return childState == 0; }
