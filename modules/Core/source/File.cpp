#include "Core/File.hpp"

#include <fstream>
#include <queue>

static void* fileThread = nullptr;
static std::atomic<bool> threadShouldStop = true;
static void* newFileRequestEvent = nullptr;

struct ReadFileAsyncRequest
{
  std::wstring path;
  Ref<ReadFileAsync> out = ReadFileAsync::create();
};
std::queue<ReadFileAsyncRequest> readFileAsyncRequests;
std::mutex readFileAsyncRequestsMutex;

bool tryReadEntireFile(const wchar_t* fileName, std::vector<byte>& buffer)
{
  TRACE_SCOPE();

  ensureTrue(fileName != nullptr, false);

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
bool tryReadFile(const wchar_t* fileName, byte* buffer, int64 sizeToRead)
{
  TRACE_SCOPE();

  ensureTrue(fileName != nullptr, false);
  ensureTrue(buffer != nullptr, false);

  std::ifstream file(fileName, std::ios::binary);
  if(!file.is_open()) {
    logError("Failed to open %S for reading.", fileName);
    return false;
  }

  file.read(reinterpret_cast<char*>(buffer), sizeToRead);

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
          logError("Failed to wait on newFileRequestEvent.");
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

      if (tryReadFile(request.path.c_str(), request.out->buffer.data, request.out->buffer.size)) // TODO: Use async/overlapping IO?
      {
        request.out->status = ReadFileAsync::Status::Success;
      }
      else
      {
        request.out->status = ReadFileAsync::Status::Error;
      }

      request.out->taskEvent->complete();
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

ReadFileAsync::Buffer::Buffer(int64 size)
{
  initialize(size);
}
ReadFileAsync::Buffer::Buffer(Buffer&& other)
{
  std::swap(data, other.data);
  std::swap(size, other.size);
}
ReadFileAsync::Buffer::~Buffer()
{
  if(data)
  {
    free(data);
    data = nullptr;
  }
}
void ReadFileAsync::Buffer::initialize(int64 inSize)
{
  if(data)
  {
    free(data);
  }
  data = (byte*)malloc(inSize);
  size = inSize;
}
Ref<ReadFileAsync> ReadFileAsync::create()
{
  return new ReadFileAsync;
}
void ReadFileAsync::ref()
{
  ++refCount;
}
void ReadFileAsync::unref()
{
  if(--refCount == 0)
  {
    delete this;
  }
}
Ref<ReadFileAsync> readFileAsync(std::wstring&& path)
{
  const int64 fileSize = getFileSize(path.c_str());
  if(!ensure(fileSize > 0))
  {
    logError("Failed to read async file due to invalid size.");
    return {};
  }

  ReadFileAsyncRequest request;
  request.out->buffer.initialize(fileSize);

  {
    std::lock_guard lock{ readFileAsyncRequestsMutex };
    readFileAsyncRequests.emplace(ReadFileAsyncRequest{ std::move(path), request.out });
  }

  SetEvent(newFileRequestEvent);

  return request.out;
}
struct ReadFileAsyncCallback
{
  Ref<ReadFileAsync> context;
  std::function<void(ReadFileAsync&)> callback;
};
DEFINE_TASK_BEGIN(readFileAsyncCallbackTask, ReadFileAsyncCallback)
{
  taskData.callback(*taskData.context.get());
}
DEFINE_TASK_END
Ref<TaskEvent> readFileAsync(std::wstring&& path, ThreadType callbackThread, std::function<void(ReadFileAsync&)>&& callback)
{
  ReadFileAsyncCallback* taskData = new ReadFileAsyncCallback();
  taskData->context = readFileAsync(std::move(path));
  taskData->callback = std::move(callback);

  return taskManager.schedule(readFileAsyncCallbackTask, taskData, callbackThread, &taskData->context->taskEvent, 1);
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

int64 getFileSize(const wchar_t* path)
{
  WIN32_FILE_ATTRIBUTE_DATA data;
  if(!ensure(GetFileAttributesEx(path, GetFileExInfoStandard, &data)))
  {
    logError("Failed to get file size for %S", path);
    return 0; 
  }
  LARGE_INTEGER size;
  size.HighPart = data.nFileSizeHigh;
  size.LowPart = data.nFileSizeLow;
  return int64(size.QuadPart);
}