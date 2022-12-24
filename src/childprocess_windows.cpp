#include "childprocess.h"

ChildProcess::~ChildProcess() {}
bool ChildProcess::Launch(const char *exec, char *const argv[]) {
  return false;
}
bool ChildProcess::Closed() const { return true; }
void ChildProcess::Write(const char *buf, size_t size) {}
void ChildProcess::NotifyTermSize(unsigned short rows, unsigned short cols) {}
void ChildProcess::HandleOutputs() {}
