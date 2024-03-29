#pragma once

#include "Core/Core.hpp"
#include "Core/Task.hpp"

#include <vector>

struct ReadFileAsync
{
  Ref<TaskEvent> taskEvent = TaskEvent::create();

  // Just a simple buffer now. In the future it may handle cases with pointer to non owned memory.
  struct Buffer
  {
    Buffer() = default;
    Buffer(int64 size);
    Buffer(const Buffer& other) = delete;
    Buffer(Buffer&& other);
    ~Buffer();

    void initialize(int64 size);

    byte* data = nullptr;
    int64 size = 0;
  };
  Buffer buffer;

  enum class Status : uint8
  {
    Unknown = 0,
    Success,
    Error
  };
  Status status = Status::Unknown;

private:

  static Ref<ReadFileAsync> create();

  void ref();
  void unref();
  std::atomic<int64> refCount = 0;

  friend struct ReadFileAsyncRequest;
  friend Ref<ReadFileAsync>;
};
Ref<ReadFileAsync> readFileAsync(std::wstring&& path);
// Returns TaskEvent of the callback having finished, not the read file having finished.
Ref<TaskEvent> readFileAsync(std::wstring&& path, ThreadType callbackThread, std::function<void(ReadFileAsync&)>&& callback);
bool tryReadEntireFile(const wchar_t* fileName, std::vector<byte>& buffer);
bool tryReadFile(const wchar_t* fileName, byte* buffer, int64 sizeToRead);

bool tryWriteFile(const wchar_t* filePath, const byte* data, int64 dataSize);

bool fileExists(const wchar_t* path);
int64 getFileSize(const wchar_t* path);

// Returns moved pointer to the beginning of path relative to current working directory.
// Returns nullptr if absolutePath does not contain the current working directory path.
const wchar_t* findPathRelativeToWorkingDirectory(const wchar_t* absolutePath);

void initializeFileSystem();
void deinitializeFileSystem();
class FileSystemGuard
{
public:

  FileSystemGuard() { initializeFileSystem(); }
  FileSystemGuard(const FileSystemGuard& other) = delete;
  FileSystemGuard(FileSystemGuard&& other) = delete;
  ~FileSystemGuard() { deinitializeFileSystem(); }
};

const wchar_t* getFileExtension(const wchar_t* fileName);
wchar_t* getFileExtension(wchar_t* fileName);
