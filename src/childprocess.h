#pragma once
#include <functional>

class ChildProcess {
  pid_t child_ = 0;
  int childfd_ = 0;

public:
  ~ChildProcess();
  std::function<void(const char *, size_t)> OutputCallback;
  bool Launch(const char *exec, char *const argv[]);
  bool Closed()const;
  void Write(const char *buf, size_t size);
  void NotifyTermSize(unsigned short rows, unsigned short cols);
  void HandleOutputs();
};
