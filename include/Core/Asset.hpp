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

  AssetDirectoryRef() = default;
  // Increases the ref count of assets in the directory and it's subdirectories, potentially causing their async loading and initialization.
  AssetDirectoryRef(const wchar_t* path);
  AssetDirectoryRef(const AssetDirectoryRef& other);
  AssetDirectoryRef(AssetDirectoryRef&& other);
  ~AssetDirectoryRef();
  AssetDirectoryRef& operator=(const AssetDirectoryRef& other);
  AssetDirectoryRef& operator=(AssetDirectoryRef&& other);

  void initialize(const wchar_t* path);

  // path is relative path from this directory.
  template<typename AssetType>
  AssetType* findAsset(const wchar_t* path) const;

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

  Ref<TaskEvent> initializedTaskEvent = TaskEvent::create();

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

#define ASSET_CLASS_BEGIN(name) \
  class name : public Asset \
  { \
    using AssetClassType = name;

#define ASSET_META_PROPERTY_EMPTY(type, name, defaultValue)
#define ASSET_META_PROPERTY_FIELD(type, name, defaultValue) type name = defaultValue;
#define ASSET_META_PROPERTY_PLUS_ONE(type, name, defaultValue) + 1

enum class AssetMetaPropertyType : uint8
{
  Unknown = 0,
  Int,
  Float,
  Enum 
  // TODO: for string to enum conversions we will probably have to create a global map like map<EnumTypeString, map<EnumValueString, Value>>;
  //       we can use std::is_unsigned<std::underlying_type_t<AssetMetaPropertyType>> to check whether the enum is signed/unsigned.
  //       or just we will have to pass it as a string initializeProperty and then parse it in initialize.
};
struct AssetMetaPropertyReflection
{
  const char* name = nullptr;
  const char* typeName = nullptr;
  uint8 size = 0;
  uint8 offset = 0;
  AssetMetaPropertyType type;
};
#define ASSET_META_PROPERTY_INITIALIZATION_REFLECTION(type, name, defaultValue) {#name, #type, sizeof(type), offsetof(InitializationProperties, name), ToAssetMetaPropertType<type>::result},
#define ASSET_META_PROPERTY_FIELD_REFLECTION(type, name, defaultValue) {#name, #type, sizeof(type), offsetof(AssetClassType, name), ToAssetMetaPropertType<type>::result},

template<typename T, class = void>
struct ToAssetMetaPropertType
{
  static constexpr AssetMetaPropertyType result = AssetMetaPropertyType::Unknown;
};
template<typename EnumType>
struct ToAssetMetaPropertType<EnumType, std::enable_if_t<std::is_enum_v<EnumType>, std::true_type>>
{
  static constexpr AssetMetaPropertyType result = AssetMetaPropertyType::Enum;
};
#define DEFINE_ToAssetMetaPropertyType(input, output) \
  template<> \
  struct ToAssetMetaPropertType<input> \
  { \
    static constexpr AssetMetaPropertyType result = AssetMetaPropertyType::output; \
  };
DEFINE_ToAssetMetaPropertyType(int8, Int)
DEFINE_ToAssetMetaPropertyType(int16, Int)
DEFINE_ToAssetMetaPropertyType(int32, Int)
DEFINE_ToAssetMetaPropertyType(int64, Int)
DEFINE_ToAssetMetaPropertyType(float, Float)

#define ASSET_CLASS_END(name) ASSET_META_PROPERTY_LIST(ASSET_META_PROPERTY_EMPTY, ASSET_META_PROPERTY_FIELD) \
    struct InitializationProperties \
    { \
      ASSET_META_PROPERTY_LIST(ASSET_META_PROPERTY_FIELD, ASSET_META_PROPERTY_EMPTY) \
    }; \
    \
    static constexpr int64 metaFieldPropertyCount = 1 ASSET_META_PROPERTY_LIST(ASSET_META_PROPERTY_EMPTY, ASSET_META_PROPERTY_PLUS_ONE) ; \
    static const AssetMetaPropertyReflection* getMetaFieldPropertyReflections() { \
      static const AssetMetaPropertyReflection fieldPropertyReflections[metaFieldPropertyCount] = { \
        {"assetType", "AssetType", sizeof(AssetType), offsetof(name, assetType), AssetMetaPropertyType::Enum}, \
        ASSET_META_PROPERTY_LIST(ASSET_META_PROPERTY_EMPTY, ASSET_META_PROPERTY_FIELD_REFLECTION) \
      }; \
      return fieldPropertyReflections; \
    } \
    \
    static constexpr int64 initializationFieldPropertyCount = 0 ASSET_META_PROPERTY_LIST(ASSET_META_PROPERTY_PLUS_ONE, ASSET_META_PROPERTY_EMPTY) ; \
    static const AssetMetaPropertyReflection* getInitializationPropertyReflections() { \
      static const AssetMetaPropertyReflection initializationPropertyReflections[initializationFieldPropertyCount + 1] = { \
        ASSET_META_PROPERTY_LIST(ASSET_META_PROPERTY_INITIALIZATION_REFLECTION, ASSET_META_PROPERTY_EMPTY) \
      }; \
      return initializationPropertyReflections; \
    } \
  };

ASSET_CLASS_BEGIN(Config)
public:

  void initialize(byte* metaData, int64 metaDataLength, byte* fileData, int64 fileDataLength, const wchar_t* fileNameExtension);

  bool getBool(const char* key) const;
  float getFloat(const char* key) const;
  double getDouble(const char* key) const;
  int64 getInt(const char* key) const;
  const std::string& getString(const char* key) const;

private:

  std::unordered_map<std::string, std::string> keysToValues;

  #define ASSET_META_PROPERTY_LIST(initializationProperty, fieldProperty)
ASSET_CLASS_END(Config)

ASSET_CLASS_BEGIN(Texture2D)
public:

  void initialize(byte* metaData, int64 metaDataLength, byte* fileData, int64 fileDataLength, const wchar_t* fileNameExtension);

  CComPtr<ID3D11ShaderResourceView> view;
  CComPtr<ID3D11Texture2D> texture;

  enum class CpuAccess : uint8
  {
    None = 0,
    Read,
    Write,
    ReadWrite
  };
  CpuAccess cpuAccess = CpuAccess::None;

  #define ASSET_META_PROPERTY_LIST(initializationProperty, fieldProperty) \
  fieldProperty(int32, width, 0) \
  fieldProperty(int32, height, 0) \
  initializationProperty(int8, mipLevelCount, 1) \
  initializationProperty(PixelFormat, pixelFormat, PixelFormat::Invalid)
ASSET_CLASS_END(Texture2D)

#define FIND_ASSET_INSTANTIATION(name) \
  template<> name* AssetDirectoryRef::findAsset<name>(const wchar_t* path) const;
ASSET_TYPE_LIST(FIND_ASSET_INSTANTIATION)
