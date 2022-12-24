#pragma once
#include <functional>

struct ChildProcess {
  std::function<void(const char *, size_t)> OutputCallback;

  ~ChildProcess();
  bool Launch(const char *exec, char *const argv[]);
  bool Closed() const;
  void Write(const char *buf, size_t size);
  void NotifyTermSize(unsigned short rows, unsigned short cols);
  void HandleOutputs();
};
