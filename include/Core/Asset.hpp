#pragma once

#include <functional>
#include <mutex>
#include <queue>

#include "Core.hpp"

class AssetManager
{
public:

  AssetManager() = default;
  AssetManager(const AssetManager& other) = delete;
  AssetManager(AssetManager&& other) = delete;
  ~AssetManager();

  bool tryInitialize();

  struct AsyncLoadResult
  {
    enum class Status
    {
      Unknown = 0,
      Success,
      Error
    } status = Status::Unknown;
    std::vector<byte> data;
  };
  void loadAsync(std::wstring&& path, std::function<void(AsyncLoadResult& result)>&& callback);

private:

  static unsigned long assetThreadMain(void* parameter);

  struct AsyncLoadRequest
  {
    std::wstring path;
    std::function<void(AsyncLoadResult& result)> callback;
  };
  std::queue<AsyncLoadRequest> asyncLoadRequests;
  std::mutex asyncLoadRequestsMutex;
  void* assetThread = nullptr;
  volatile bool threadShouldStop = false;
  void* newAsyncLoadRequestEvent = nullptr;
};

extern AssetManager assetManager;