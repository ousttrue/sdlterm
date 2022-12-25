#pragma once
#include <functional>

struct ChildProcess {
  char ReadBuffer[8192];

  ~ChildProcess();
  bool Launch(const char *exec, char *const argv[]);
  bool Closed() const;
  void Write(const char *buf, size_t size);
  void NotifyTermSize(unsigned short rows, unsigned short cols);
  const char *Read(size_t *pSize);
};
