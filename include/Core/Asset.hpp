#pragma once

#include <functional>

#include "Core/Core.hpp"
#include "Core/Memory.hpp"
#include "Core/Task.hpp"
#include "Core/Image.hpp"
#include "Core/D3D11.hpp"

bool tryInitializeAssetSystem();

class AssetDirectory;
class AssetDirectoryRef
{
public:

  AssetDirectoryRef() = default;
  // Increases the ref count of assets in the directory and it's subdirectories, potentially causing their async loading and initialization.
  AssetDirectoryRef(const wchar_t* path);
  AssetDirectoryRef(const AssetDirectoryRef& other) = delete;
  AssetDirectoryRef(AssetDirectoryRef&& other);
  ~AssetDirectoryRef();

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

  Asset() {};
  Asset(const Asset& other) = delete;
  Asset(Asset&& other) = delete;

public:

  const wchar_t* path;
  Ref<TaskEvent> initializedTaskEvent = TaskEvent::create();
  AssetType assetType;
  union // Prevents initialization of refcount value
  {
    std::atomic<int32> refCount;
  };

  void ref();
  void unref();  // Call destructor based on type if refCount reaches 0.

};
Asset* internalFindAsset(AssetDirectory* directory, const wchar_t* path);

#define ASSET_CLASS_BEGIN(name) \
  class name : public Asset \
  { \
    using AssetClassType = name;

#define ASSET_META_PROPERTY_EMPTY(type, name, defaultValue)
#define ASSET_META_PROPERTY_FIELD(type, name, defaultValue) type name;
#define ASSET_META_PROPERTY_PLUS_ONE(type, name, defaultValue) + 1

enum class AssetMetaPropertyType : uint8
{
  Unknown = 0,
  Int8,
  Int16,
  Int32,
  Int64,
  Uint8,
  Uint16,
  Uint32,
  Uint64,
  Bool,
  Float,
  SignedEnum,
  UnsignedEnum
};
struct AssetMetaPropertyReflection
{
  const char* name = nullptr;
  const char* typeName = nullptr;
  uint64 defaultValue = 0;
  uint8 size = 0;
  uint8 offset = 0;
  AssetMetaPropertyType type;
};
#define ASSET_META_PROPERTY_INITIALIZATION_REFLECTION(type, name, defaultValue) {#name, #type, uint64(defaultValue), sizeof(type), offsetof(InitializationProperties, name), ToAssetMetaPropertType<type>::result},
#define ASSET_META_PROPERTY_FIELD_REFLECTION(type, name, defaultValue) {#name, #type, uint64(defaultValue), sizeof(type), offsetof(AssetClassType, name), ToAssetMetaPropertType<type>::result},

template<typename T, class = void>
struct ToAssetMetaPropertType
{
  static constexpr AssetMetaPropertyType result = AssetMetaPropertyType::Unknown;
};
template<typename EnumType>
struct ToAssetMetaPropertType<EnumType, typename std::enable_if<std::is_enum_v<EnumType>>::type>
{
  static constexpr AssetMetaPropertyType result = std::is_signed<std::underlying_type_t<EnumType>>::value ? 
    AssetMetaPropertyType::SignedEnum : AssetMetaPropertyType::UnsignedEnum;
};
#define DEFINE_ToAssetMetaPropertyType(input, output) \
  template<> \
  struct ToAssetMetaPropertType<input> \
  { \
    static constexpr AssetMetaPropertyType result = AssetMetaPropertyType::output; \
  };
DEFINE_ToAssetMetaPropertyType(int8, Int8)
DEFINE_ToAssetMetaPropertyType(int16, Int16)
DEFINE_ToAssetMetaPropertyType(int32, Int32)
DEFINE_ToAssetMetaPropertyType(int64, Int64)
DEFINE_ToAssetMetaPropertyType(uint8, Uint8)
DEFINE_ToAssetMetaPropertyType(uint16, Uint16)
DEFINE_ToAssetMetaPropertyType(uint32, Uint32)
DEFINE_ToAssetMetaPropertyType(uint64, Uint64)
DEFINE_ToAssetMetaPropertyType(bool, Bool)
DEFINE_ToAssetMetaPropertyType(float, Float)

#define ASSET_CLASS_END(name) public: ASSET_META_PROPERTY_LIST(ASSET_META_PROPERTY_FIELD) \
    void initialize(const byte* fileData, int64 fileDataLength); \
    \
    static constexpr int64 metaPropertyCount = 0 ASSET_META_PROPERTY_LIST(ASSET_META_PROPERTY_PLUS_ONE) ; \
    static const AssetMetaPropertyReflection* getMetaPropertyReflections() { \
      static const AssetMetaPropertyReflection PropertyReflections[metaPropertyCount + 1] = { \
        ASSET_META_PROPERTY_LIST(ASSET_META_PROPERTY_FIELD_REFLECTION) \
      }; \
      return PropertyReflections; \
    } \
    \
  };

ASSET_CLASS_BEGIN(Config)
public:

  bool getBool(const char* key) const;
  float getFloat(const char* key) const;
  double getDouble(const char* key) const;
  int64 getInt(const char* key) const;
  const std::string& getString(const char* key) const;

private:

  std::unordered_map<std::string, std::string> keysToValues;

  #define ASSET_META_PROPERTY_LIST(Property)
ASSET_CLASS_END(Config)

ASSET_CLASS_BEGIN(Texture2D)
public:

  CComPtr<ID3D11ShaderResourceView> view;
  CComPtr<ID3D11Texture2D> texture;

  #define ASSET_META_PROPERTY_LIST(Property) \
  Property(int32, width, 0) \
  Property(int32, height, 0) \
  Property(int8, mipLevelCount, 1) \
  Property(bool, cpuAccess, false) \
  Property(PixelFormat, pixelFormat, PixelFormat::Invalid)

  template<typename PixelFormatValueType>
  PixelFormatValueType sample(int64 x, int64 y) const;

private:

  std::vector<byte> cpuData;

ASSET_CLASS_END(Texture2D)
template<typename PixelFormatValueType>
PixelFormatValueType Texture2D::sample(int64 x, int64 y) const
{
  const int64 bytesPerPixel = toPixelSizeInBytes(pixelFormat);
  ensure(sizeof(PixelFormatValueType) == bytesPerPixel);

  const PixelFormatValueType* cpuDataTyped = (const PixelFormatValueType*)cpuData.data();
  return cpuDataTyped[y * width + x];
}

#define FIND_ASSET_INSTANTIATION(name) \
  template<> name* AssetDirectoryRef::findAsset<name>(const wchar_t* path) const;
ASSET_TYPE_LIST(FIND_ASSET_INSTANTIATION)
