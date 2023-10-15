#include "Core/File.hpp"

#include <fstream>
#include <queue>

static void* fileThread = nullptr;
static std::atomic<bool> threadShouldStop = true;
static void* newFileRequestEvent = nullptr;

struct ReadFileAsyncRequest
{
  std::wstring path;
  std::function<void(ReadFileAsyncResult& result)> callback;
};
std::queue<ReadFileAsyncRequest> readFileAsyncRequests;
std::mutex readFileAsyncRequestsMutex;

bool tryReadEntireFile(const wchar_t* fileName, std::vector<byte>& buffer)
{
  TRACE_SCOPE();

  std::ifstream file(fileName, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    logError("Failed to open %S for reading.", fileName);
    return false;
  }

  const size_t fileSize = file.tellg();
  buffer.resize(fileSize);

  file.seekg(0);
  file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

  return true;
}

unsigned long fileThreadMain(void* parameter)
{
  TRACE_THREAD("FileThread");

  while (!threadShouldStop)
  {
    {
      TRACE_SCOPE("waitForRequests");

      DWORD waitResult = WaitForSingleObject(newFileRequestEvent, INFINITE);
      switch (waitResult)
      {
        case WAIT_FAILED:
          logError("Failed to wait on parallelForFinishedEvent.");
          continue;

        default:
          break;
      }

      if (threadShouldStop)
      {
        return 0;
      }
    }

    std::size_t remainingRequests = 0;
    do
    {
      ReadFileAsyncRequest request;
      {
        TRACE_SCOPE("popRequest");
        {
          std::lock_guard lock{ readFileAsyncRequestsMutex };
          request = std::move(readFileAsyncRequests.front());
          readFileAsyncRequests.pop();
          remainingRequests = readFileAsyncRequests.size();
        }
      }

      ReadFileAsyncResult result;
      if (tryReadEntireFile(request.path.c_str(), result.data)) // TODO: Use async/overlapping IO?
      {
        result.status = ReadFileAsyncResult::Status::Success;
      }
      else
      {
        result.status = ReadFileAsyncResult::Status::Error;
      }

      {
        TRACE_SCOPE("readFileAsyncCallback");
        request.callback(result);
      }
    } while (remainingRequests > 0 && !threadShouldStop);
  }

  return 0;
}

void initializeFileSystem()
{
  threadShouldStop = false;

  fileThread = CreateThread(NULL, 0, &fileThreadMain, nullptr, 0, NULL);
  if (!fileThread)
  {
    logError("Failed to create file thread.");
  }

  newFileRequestEvent = CreateEvent(NULL, false, false, NULL);
  if (!newFileRequestEvent)
  {
    logError("Failed to create file request event");
  }
}

void deinitializeFileSystem()
{
  threadShouldStop = true;

  if (fileThread)
  {
    if (newFileRequestEvent)
    {
      SetEvent(newFileRequestEvent);
    }

    constexpr DWORD waitTimeoutMs = 1000;
    DWORD waitResult = WaitForSingleObject(fileThread, waitTimeoutMs);
    switch (waitResult)
    {
      case WAIT_TIMEOUT:
        logError("FileThread %d stop timeout %d ms.", GetThreadId(fileThread), waitTimeoutMs);
        break;

      case WAIT_FAILED:
        logError("FileThread %d stop failed.", GetThreadId(fileThread));
        break;

      default:
        CloseHandle(fileThread);
        break;
    }
  }

  if (newFileRequestEvent)
  {
    CloseHandle(newFileRequestEvent);
  }
}

void readFileAsync(std::wstring&& path, std::function<void(ReadFileAsyncResult& result)>&& callback)
{
  {
    std::lock_guard lock{ readFileAsyncRequestsMutex };
    readFileAsyncRequests.emplace(ReadFileAsyncRequest{ std::move(path), std::move(callback) });
  }

  SetEvent(newFileRequestEvent);
}

bool tryWriteFile(const wchar_t* filePath, const byte* data, int64 dataSize)
{
  HANDLE file = CreateFile(filePath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file == INVALID_HANDLE_VALUE)
  {
    return false;
  }

  DWORD bytesWritten;
  if (!WriteFile(file, data, dataSize, &bytesWritten, nullptr))
  {
    CloseHandle(file);
    return false;
  }

  if(bytesWritten < dataSize)
  {
    CloseHandle(file);
    return false;
  }

  CloseHandle(file);
  return true;
}

bool fileExists(const wchar_t* path)
{
  return GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
}