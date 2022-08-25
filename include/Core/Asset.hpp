#pragma once

#include <functional>
#include <mutex>
#include <queue>

#include "Core.hpp"
#include "Core/Memory.hpp"

// TODO: create an AssetRegistry inside the manager, that knows about all assets in the asset folder.
// The registry will be filled during game start on the asset thread. 
// The registry data structure will be static for the lifetime of the process, so no need to guard it against concurrent access,
// although its content (the individual asset classes) may be accessed concurrently (TODO: do we really need this?).
// The data structure will be organized into a tree, where a file directory is a node.
// The user code can then request to load all assets from the subtree. He will then hold an AssetDirectory reference,
// which will make all the underlying assets reference count increase by 1 to keep them loaded.
// For example in case there are 3 subtrees and two of them want to share an asset, one can contain a system shortcut to the other.
// Individual assets/resources (Texture2D, Config) will be initialized based on a meta file that will be present next to the loaded asset file.
// That meta file is separate from the asset file to allow VCS diffs. The meta file will be an INI file

// TODO: Integrate libconfini

// TODO: Asset types could be defines in a AssetType.inl with macros, where the macro can once be used here to define enum values and also in ~Asset()
enum class AssetType : uint16
{
  Unknown = 0,
  Config = 1,
  Texture2D = 100,
  VertexShader = 200,
  PixelShader = 201
};

class Asset
{

public:

  AssetType getAssetType() const { return type; }

  void ref();
  void unref();

protected:

  Asset() = default;
  Asset(const Asset& other) = delete;
  Asset(Asset&& other) = delete;
  ~Asset(); // Call destructor based on type.

private:

  std::atomic<int32> refCount = 0;
  AssetType type = AssetType::Unknown;
};

class Texture2D : public Asset
{
public:

  static Ref<Texture2D> create();

};

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
  std::queue<AsyncLoadRequest> asyncLoadRequests; // TODO: use MPSCQueue ?
  std::mutex asyncLoadRequestsMutex;
  void* assetThread = nullptr;
  volatile bool threadShouldStop = false;
  void* newAsyncLoadRequestEvent = nullptr;
};

extern AssetManager assetManager;