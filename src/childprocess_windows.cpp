#include "childprocess.h"
#include <Windows.h>
#include <algorithm>
#include <mutex>
#include <process.h>
#include <stdexcept>

// Forward declarations
HRESULT InitializeStartupInfoAttachedToPseudoConsole(STARTUPINFOEXA *, HPCON);

struct Context {
  std::vector<char> buffer_;
  std::mutex mtx_;
  HANDLE inpipe_ = nullptr;
  HANDLE outpipe_ = nullptr;

  void Enqueue(const char *buf, DWORD len) {
    if (len == 0) {
      return;
    }
    std::lock_guard<std::mutex> lock(mtx_);
    auto size = buffer_.size();
    buffer_.resize(size + len);
    memcpy(buffer_.data() + size, buf, len);
  }

  DWORD Dequeue(char *buf, size_t len) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (len < buffer_.size()) {
      throw std::runtime_error("not implemented yet");
    } else {
      auto size = std::min(len, buffer_.size());
      memcpy(buf, buffer_.data(), size);
      buffer_.resize(0);
      return size;
    }
  }
};
Context context_;

static void __cdecl PipeListener(LPVOID p) {
  auto context = (Context *)p;
  HANDLE hPipe{context->inpipe_};
  HANDLE hConsole{GetStdHandle(STD_OUTPUT_HANDLE)};

  const DWORD BUFF_SIZE{512};
  char szBuffer[BUFF_SIZE]{};

  DWORD dwBytesWritten{};
  DWORD dwBytesRead{};
  BOOL fRead{FALSE};
  do {
    // Read from the pipe
    fRead = ReadFile(hPipe, szBuffer, BUFF_SIZE, &dwBytesRead, NULL);

    // Write received text to the Console
    // Note: Write to the Console using WriteFile(hConsole...), not
    // printf()/puts() to prevent partially-read VT sequences from corrupting
    // output
    // WriteFile(hConsole, szBuffer, dwBytesRead, &dwBytesWritten, NULL);
    context->Enqueue(szBuffer, BUFF_SIZE);

  } while (fRead && dwBytesRead >= 0);
}

static HRESULT CreatePseudoConsoleAndPipes(HPCON *phPC, HANDLE *phPipeIn,
                                           HANDLE *phPipeOut) {
  HRESULT hr{E_UNEXPECTED};
  HANDLE hPipePTYIn{INVALID_HANDLE_VALUE};
  HANDLE hPipePTYOut{INVALID_HANDLE_VALUE};

  // Create the pipes to which the ConPTY will connect
  if (CreatePipe(&hPipePTYIn, phPipeOut, NULL, 0) &&
      CreatePipe(phPipeIn, &hPipePTYOut, NULL, 0)) {
    // Determine required size of Pseudo Console
    COORD consoleSize{};
    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    HANDLE hConsole{GetStdHandle(STD_OUTPUT_HANDLE)};
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
      consoleSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
      consoleSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }

    // Create the Pseudo Console of the required size, attached to the PTY-end
    // of the pipes
    hr = CreatePseudoConsole(consoleSize, hPipePTYIn, hPipePTYOut, 0, phPC);

    // Note: We can close the handles to the PTY-end of the pipes here
    // because the handles are dup'ed into the ConHost and will be released
    // when the ConPTY is destroyed.
    if (INVALID_HANDLE_VALUE != hPipePTYOut)
      CloseHandle(hPipePTYOut);
    if (INVALID_HANDLE_VALUE != hPipePTYIn)
      CloseHandle(hPipePTYIn);
  }

  return hr;
}

// Initializes the specified startup info struct with the required properties
// and updates its thread attribute list with the specified ConPTY handle
HRESULT
InitializeStartupInfoAttachedToPseudoConsole(STARTUPINFOEXA *pStartupInfo,
                                             HPCON hPC) {
  HRESULT hr{E_UNEXPECTED};

  if (pStartupInfo) {
    size_t attrListSize{};

    pStartupInfo->StartupInfo.cb = sizeof(STARTUPINFOEX);

    // Get the size of the thread attribute list.
    InitializeProcThreadAttributeList(NULL, 1, 0, &attrListSize);

    // Allocate a thread attribute list of the correct size
    pStartupInfo->lpAttributeList =
        reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(malloc(attrListSize));

    // Initialize thread attribute list
    if (pStartupInfo->lpAttributeList &&
        InitializeProcThreadAttributeList(pStartupInfo->lpAttributeList, 1, 0,
                                          &attrListSize)) {
      // Set Pseudo Console attribute
      hr = UpdateProcThreadAttribute(pStartupInfo->lpAttributeList, 0,
                                     PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, hPC,
                                     sizeof(HPCON), NULL, NULL)
               ? S_OK
               : HRESULT_FROM_WIN32(GetLastError());
    } else {
      hr = HRESULT_FROM_WIN32(GetLastError());
    }
  }
  return hr;
}

ChildProcess::~ChildProcess() {}
bool ChildProcess::Launch(const char *exec, char *const argv[]) {
  HPCON hPC{INVALID_HANDLE_VALUE};

  //  Create the Pseudo Console and pipes to it
  // HANDLE hPipeIn{INVALID_HANDLE_VALUE};
  auto hr =
      CreatePseudoConsoleAndPipes(&hPC, &context_.inpipe_, &context_.outpipe_);
  if (FAILED(hr)) {
    return false;
  }

  // Create & start thread to listen to the incoming pipe
  // Note: Using CRT-safe _beginthread() rather than CreateThread()
  HANDLE hPipeListenerThread{
      reinterpret_cast<HANDLE>(_beginthread(PipeListener, 0, &context_))};

  // Initialize the necessary startup info struct
  STARTUPINFOEXA startupInfo{};
  if (FAILED(InitializeStartupInfoAttachedToPseudoConsole(&startupInfo, hPC))) {
    return false;
  }

  // Launch ping to emit some text back via the pipe
  PROCESS_INFORMATION piClient{};
  hr = CreateProcessA(NULL,         // No module name - use Command Line
                      (char *)exec, // Command Line
                      NULL,         // Process handle not inheritable
                      NULL,         // Thread handle not inheritable
                      FALSE,        // Inherit handles
                      EXTENDED_STARTUPINFO_PRESENT, // Creation flags
                      NULL, // Use parent's environment block
                      NULL, // Use parent's starting directory
                      &startupInfo.StartupInfo, // Pointer to STARTUPINFO
                      &piClient) // Pointer to PROCESS_INFORMATION
           ? S_OK
           : GetLastError();

  // if (S_OK == hr) {
  //   // Wait up to 10s for ping process to complete
  //   WaitForSingleObject(piClient.hThread, 10 * 1000);

  //   // Allow listening thread to catch-up with final output!
  //   Sleep(500);
  // }

  // --- CLOSEDOWN ---

  // // Now safe to clean-up client app's process-info & thread
  // CloseHandle(piClient.hThread);
  // CloseHandle(piClient.hProcess);

  // // Cleanup attribute list
  // DeleteProcThreadAttributeList(startupInfo.lpAttributeList);
  // free(startupInfo.lpAttributeList);

  // Close ConPTY - this will terminate client process if running
  // ClosePseudoConsole(hPC);

  // Clean-up the pipes
  // if (INVALID_HANDLE_VALUE != hPipeOut)
  //   CloseHandle(hPipeOut);
  // if (INVALID_HANDLE_VALUE != hPipeIn)
  //   CloseHandle(hPipeIn);

  return true;
}
bool ChildProcess::Closed() const { return false; }
void ChildProcess::Write(const char *buf, size_t size) {
  DWORD write_size;
  WriteFile(context_.outpipe_, buf, size, &write_size, NULL);
}
void ChildProcess::NotifyTermSize(unsigned short rows, unsigned short cols) {
  // TODO
}
const char *ChildProcess::Read(size_t *pSize) {
  *pSize = context_.Dequeue(ReadBuffer, sizeof(ReadBuffer));
  return ReadBuffer;
}
