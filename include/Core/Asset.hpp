#pragma once

#include <functional>
#include <mutex>
#include <queue>

#include "Core/Core.hpp"
#include "Core/Memory.hpp"
#include "Core/Task.hpp"

// TODO: create an AssetRegistry, that knows about all assets in the asset folder.
// The registry will be filled during game start. 
// The registry data structure will be static for the lifetime of the process, so no need to guard it against concurrent access,
// although its content (the individual asset classes) may be accessed concurrently (TODO: do we really need this?).
// The data structure will be organized into a tree, where a file directory is a node.
// The user code can then request to load all assets from the subtree. He will then hold an AssetDirectory reference,
// which will make all the underlying assets reference count increase by 1 to keep them loaded.
// For example in case there are 3 subtrees and two of them want to share an asset, one can contain a system shortcut to the other.
// Individual assets/resources (Texture2D, Config) will be initialized based on a meta file that will be present next to the loaded asset file.
// That meta file is separate from the asset file to allow VCS diffs. The meta file will be an INI file

void initializeAssetSystem();

class AssetDirectoryRef
{
public:

  AssetDirectoryRef(const AssetDirectoryRef& other);
  AssetDirectoryRef(AssetDirectoryRef&& other);
  ~AssetDirectoryRef();
  AssetDirectoryRef& operator=(const AssetDirectoryRef& other);
  AssetDirectoryRef& operator=(AssetDirectoryRef&& other);

  // Completed when all assets in asset directory subtree are loaded and initialized.
  Ref<TaskEvent> loadedEvent;

private:

  AssetDirectoryRef(class AssetDirectory* inDirectory) : directory(inDirectory) {}
  friend AssetDirectoryRef loadAssetDirectory(const wchar_t* path);

  class AssetDirectory* directory = nullptr;
};
// Increases the ref count of assets in the directory and it's subdirectories, potentially causing their async loading and initialization.
AssetDirectoryRef loadAssetDirectory(const wchar_t* path);

// TODO: Asset types could be defines in a AssetType.inl with macros, where the macro can once be used here to define enum values and also in ~Asset()
enum class AssetType : uint16
{
  Unknown = 0,
  Config = 1,
  Texture2D = 100,
};

class Asset
{

public:

  AssetType assetType = AssetType::Unknown;

  void ref();
  void unref();

protected:

  Asset() = default;
  Asset(const Asset& other) = delete;
  Asset(Asset&& other) = delete;
  ~Asset(); // Call destructor based on type.

private:

  std::atomic<int32> refCount = 0;
};

class Texture2D : public Asset
{
public:

  static Ref<Texture2D> create();

};
