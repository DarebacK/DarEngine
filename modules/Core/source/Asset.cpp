#include "Core/Asset.hpp"

#include "Core/File.hpp"

AssetManager assetManager;

AssetManager::~AssetManager()
{
  threadShouldStop = true;

  if (assetThread)
  {
    SetEvent(newAsyncLoadRequestEvent);

    constexpr DWORD waitTimeoutMs = 1000;
    DWORD waitResult = WaitForSingleObject(assetThread, waitTimeoutMs);
    switch (waitResult)
    {
      case WAIT_TIMEOUT:
        logError("AssetThread %d stop timeout %d ms.", GetThreadId(assetThread), waitTimeoutMs);
        break;

      case WAIT_FAILED:
        logError("AssetThread %d stop failed.", GetThreadId(assetThread));
        break;

      default:
        CloseHandle(assetThread);
        break;
    }
  }

  if (newAsyncLoadRequestEvent)
  {
    CloseHandle(newAsyncLoadRequestEvent);
  }
}

bool AssetManager::tryInitialize()
{
  assetThread = CreateThread(NULL, 0, &assetThreadMain, this, 0, NULL);
  if (!assetThread)
  {
    logError("Failed to create AssetThread");
    return false;
  }

  newAsyncLoadRequestEvent = CreateEvent(NULL, false, false, NULL);
  if (!newAsyncLoadRequestEvent)
  {
    logError("Failed to create async load request event");
    return false;
  }

  return true;
}

unsigned long AssetManager::assetThreadMain(void* parameter)
{
  TRACE_THREAD("AssetThread");

  AssetManager& assetManager = *static_cast<AssetManager*>(parameter);

  while (!assetManager.threadShouldStop)
  {
    {
      TRACE_SCOPE("WaitForRequests");

      DWORD waitResult = WaitForSingleObject(assetManager.newAsyncLoadRequestEvent, INFINITE);
      switch (waitResult)
      {
        case WAIT_FAILED:
          logError("Failed to wait on parallelForFinishedEvent.");
          continue;

        default:
          break;
      }
    }


    std::size_t remainingRequests = 0;
    do
    {
      AsyncLoadRequest request;
      {
        TRACE_SCOPE("PopRequest");
        {
          std::lock_guard lock{ assetManager.asyncLoadRequestsMutex };
          request = std::move(assetManager.asyncLoadRequests.front());
          assetManager.asyncLoadRequests.pop();
          remainingRequests = assetManager.asyncLoadRequests.size();
        }
      }

      AsyncLoadResult result;
      if (tryReadEntireFile(request.path.c_str(), result.data)) // TODO: Use async/overlapping IO?
      {
        result.status = AsyncLoadResult::Status::Success;
      }
      else
      {
        result.status = AsyncLoadResult::Status::Error;
      }

      {
        TRACE_SCOPE("RequestCallback");
        request.callback(result);
      }
    } while (remainingRequests > 0 && assetManager.threadShouldStop);
  }

  return 0;
}

void AssetManager::loadAsync(std::wstring&& path, std::function<void(AsyncLoadResult& result)>&& callback)
{
  {
    std::lock_guard lock{ asyncLoadRequestsMutex };
    asyncLoadRequests.emplace(AsyncLoadRequest{ std::move(path), std::move(callback) });
  }

  SetEvent(newAsyncLoadRequestEvent);
}