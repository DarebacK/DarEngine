#pragma once

#include <functional>
#include <mutex>
#include <queue>

#include "Core/Core.hpp"
#include "Core/Memory.hpp"
#include "Core/Task.hpp"
#include "Core/Image.hpp"
#include "Core/D3D11.hpp"

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

bool tryInitializeAssetSystem();

class AssetDirectory;
class AssetDirectoryRef
{
public:

  // Increases the ref count of assets in the directory and it's subdirectories, potentially causing their async loading and initialization.
  AssetDirectoryRef(const wchar_t* path);
  AssetDirectoryRef(const AssetDirectoryRef& other);
  AssetDirectoryRef(AssetDirectoryRef&& other);
  ~AssetDirectoryRef();
  AssetDirectoryRef& operator=(const AssetDirectoryRef& other);
  AssetDirectoryRef& operator=(AssetDirectoryRef&& other);

  // path is relative path from this directory.
  template<typename AssetType>
  AssetType* findAsset(const wchar_t* path) const;

  // Completed when all assets in asset directory subtree are loaded and initialized.
  Ref<TaskEvent> loadedEvent;

private:

  AssetDirectory* directory = nullptr;
};

#define ASSET_TYPE_LIST(macro) \
  macro(Config) \
  macro(Texture2D)
enum class AssetType : uint16
{
  Unknown = 0,
#define ASSET_TYPE_ENUM(name) name,
  ASSET_TYPE_LIST(ASSET_TYPE_ENUM)
#undef ASSET_TYPE_ENUM
};
const char* toString(AssetType type);

class Asset
{
protected:

  Asset() = default;
  Asset(const Asset& other) = delete;
  Asset(Asset&& other) = delete;

public:

  // Used to delete itself from the asset directory after ref count reaches zero.
  // TODO: maybe we don't have to delete the assets? Just call destructor/constructor without deletion.
  Asset** pointerInAssetDirectory = nullptr;
  AssetType assetType = AssetType::Unknown;

  void ref();
  void unref();  // Call destructor based on type if refCount reaches 0.

private:

  std::atomic<int32> refCount = 0;
};
Asset* internalFindAsset(AssetDirectory* directory, const wchar_t* path);

class Config : public Asset
{
public:

  void initialize(byte* metaData, int64 metaDataLength, byte* fileData, int64 fileDataLength);

  bool getBool(const char* key) const;
  float getFloat(const char* key) const;
  double getDouble(const char* key) const;
  int64 getInt(const char* key) const;
  const std::string& getString(const char* key) const;

private:

  std::unordered_map<std::string, std::string> keysToValues;
};

class Texture2D : public Asset
{
public:

  void initialize(byte* metaData, int64 metaDataLength, byte* fileData, int64 fileDataLength);

private:

  CComPtr<ID3D11ShaderResourceView> view;
  CComPtr<ID3D11Texture2D> texture;

  int32 width = 0;
  int32 height = 0;
  int8 mipLevelCount = 1;
  PixelFormat pixelFormat = PixelFormat::Invalid;
  enum class CpuAccess : uint8
  {
    None = 0,
    Read,
    Write,
    ReadWrite
  };
  CpuAccess cpuAccess = CpuAccess::None;
};

#define FIND_ASSET_INSTANTIATION(name) \
  template<> name* AssetDirectoryRef::findAsset<name>(const wchar_t* path) const;
ASSET_TYPE_LIST(FIND_ASSET_INSTANTIATION)