#pragma once

#include "Core/Core.hpp"
#include "Core/Task.hpp"

#include <vector>

bool tryReadEntireFile(const wchar_t* fileName, std::vector<byte>& buffer);

struct ReadFileAsyncResult
{
  enum class Status
  {
    Unknown = 0,
    Success,
    Error
  } status = Status::Unknown;
  std::vector<byte> data;
};
// Callback is called on the file thread. Don't do much of work there to not delay any additional IO.
void readFileAsync(std::wstring&& path, std::function<void(ReadFileAsyncResult& result)>&& callback);

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