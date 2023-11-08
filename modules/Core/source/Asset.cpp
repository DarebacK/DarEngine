#define DAR_MODULE_NAME "Asset"

#include "Core/Asset.hpp"

#include <string>
#include <algorithm>

#include "Core/File.hpp"
#include "Core/String.hpp"
#include "Core/Config.hpp"

#define FIND_ASSET_IMPLEMENTATION(name) \
  template<> \
  name* AssetDirectoryRef::findAsset<name>(const wchar_t* path) const \
  { \
    TRACE_SCOPE() \
    Asset* asset = internalFindAsset(directory, path); \
    if(!ensure(asset != nullptr)) return nullptr; \
    if(!ensure(asset->assetType == AssetType::name)) return nullptr; \
    return reinterpret_cast<name*>(asset); \
  }
ASSET_TYPE_LIST(FIND_ASSET_IMPLEMENTATION)

const char* toString(AssetType type)
{
#define ASSET_TYPE_ENUMTOSTRING(name) case AssetType::name: return #name;
  switch (type)
  {
    ASSET_TYPE_LIST(ASSET_TYPE_ENUMTOSTRING)
  default:
    ensureNoEntry();
    return "Unknown";
  }
}

AssetType assetTypeStringToEnum(const char* string)
{
#define ASSET_TYPE_STRINGTOENUM(name) \
  else if(isEqual(string, #name)) \
  { \
    return AssetType::name; \
  }

  if(false) { }
  ASSET_TYPE_LIST(ASSET_TYPE_STRINGTOENUM)
#undef ASSET_TYPE_STRINGTOENUM

  return AssetType::Unknown;
}

static const wchar_t* getFileExtension(const wchar_t* fileName)
{
  int32 characterIndex = 0;
  int32 lastDotIndex = -1;
  while (fileName[characterIndex] != L'\0')
  {
    wchar_t character = fileName[characterIndex];
    switch (character)
    {
    case L'.': lastDotIndex = characterIndex;
    }
    characterIndex++;
  }

  if (lastDotIndex <= 0 || (lastDotIndex == characterIndex - 1))
  {
    return nullptr;
  }

  return fileName + lastDotIndex + 1;
}
static wchar_t* getFileExtension(wchar_t* fileName)
{
  return const_cast<wchar_t*>(getFileExtension(const_cast<const wchar_t*>(fileName)));
}

DEFINE_TASK_BEGIN(initializeAsset, Asset)
{
  taskDataGuard.release();

  // Use memory mapped file to avoid unnecessary copy when passing the resulting to a buffer, 
  // e.g. during ID3D11Device::CreateTexture2D.

  HANDLE fileHandle;
  HANDLE fileMapping;
  void* fileView;
  int64 fileSize;

  Asset& assetBase = taskData;

  {
    TRACE_SCOPE("mapViewOfAssetFile");

    fileHandle = CreateFile(assetBase.path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if(!ensure(fileHandle))
    {
      logError("Failed to load %S asset file", assetBase.path);
      return;
    }

    fileMapping = CreateFileMapping(fileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
    if(!ensure(fileMapping))
    {
      logError("Failed to create file mapping for %S asset file", assetBase.path);
      CloseHandle(fileHandle);
      return;
    }

    fileView = MapViewOfFile(fileMapping, FILE_MAP_READ, 0, 0, 0);
    if(!ensure(fileView))
    {
      CloseHandle(fileMapping);
      CloseHandle(fileHandle);
      logError("Failed to create map view for %S asset file", assetBase.path);
      return;
    }

    DWORD fileSizeHigh;
    DWORD fileSizeLow = GetFileSize(fileHandle, &fileSizeHigh);
    fileSize = int64(uint64(fileSizeLow) | (uint64(fileSizeHigh) << 32));
  }

  switch(assetBase.assetType)
  {
    #define ASSET_TYPE_INITIALIZE(name) \
          case AssetType::name: { \
            TRACE_SCOPE(#name "::initialize"); \
            name& asset = reinterpret_cast<name&>(assetBase); \
            asset.initialize((const byte*)fileView, fileSize); \
            asset.initializedTaskEvent->complete(); \
              break; \
          }

    ASSET_TYPE_LIST(ASSET_TYPE_INITIALIZE)
      #undef ASSET_TYPE_INITIALIZE

    default:
      ensureNoEntry();
      break;
  }

  TRACE_SCOPE("unmapViewOfAssetFile");
  // This can take couple of milliseconds. 
  // initializedTaskEvent was set to complete so it's no problem we do it as part of this task.
  UnmapViewOfFile(fileView);
  CloseHandle(fileMapping);
  CloseHandle(fileHandle);
}
DEFINE_TASK_END

void Asset::ref()
{
  if(++refCount == 1)
  {
    TRACE_SCOPE("constructAsset");

    ensureTrue(isInMainThread());

    switch(assetType)
    {
      #define ASSET_TYPE_CONSTRUCT_CASE(name) case AssetType::name: { \
      name* derivedPtr = reinterpret_cast<name*>(this); \
      new (derivedPtr) name; \
      break; }

      ASSET_TYPE_LIST(ASSET_TYPE_CONSTRUCT_CASE)
      #undef ASSET_TYPE_CONSTRUCT_CASE

      default:
        ensureNoEntry();
        break;
    }

    taskManager.schedule(initializeAsset, this, ThreadType::Worker);
  }
}

void Asset::unref()
{
  if (--refCount == 0)
  {
    TRACE_SCOPE("destructAsset");

    ensureTrue(isInMainThread());

    switch (assetType)
    {
    #define ASSET_TYPE_DELETE_CASE(name) case AssetType::name: {\
      name* derivedPtr = reinterpret_cast<name*>(this); \
      derivedPtr->~name(); \
      break; }
    
      ASSET_TYPE_LIST(ASSET_TYPE_DELETE_CASE)
    #undef ASSET_TYPE_DELETE_CASE

      default:
        ensureNoEntry();
        break;
    }
  }
}

bool parseMetaProperty(const AssetMetaPropertyReflection* reflections, int64 reflectionCount, const ConfigKeyValueNode& node, void* destination)
{
  TRACE_SCOPE();

  for(int64 propertyIndex = 0; propertyIndex < reflectionCount; ++propertyIndex)
  {
    const AssetMetaPropertyReflection& reflection = reflections[propertyIndex];
    if(isEqual(node.key, reflection.name))
    {
      switch(reflection.type)
      {
        #define REFLECTION_ASSIGN_INT_CASE(MetaPropertyType, ValueType) \
          case AssetMetaPropertyType::MetaPropertyType: \
          { \
            ValueType value = (ValueType)node.toInt(); \
            *(ValueType*)(((byte*)destination) + reflection.offset) = value; \
            break; \
          }
        REFLECTION_ASSIGN_INT_CASE(Int8, int8)
        REFLECTION_ASSIGN_INT_CASE(Int16, int16)
        REFLECTION_ASSIGN_INT_CASE(Int32, int32)
        REFLECTION_ASSIGN_INT_CASE(Int64, int64)
        REFLECTION_ASSIGN_INT_CASE(Uint8, uint8)
        REFLECTION_ASSIGN_INT_CASE(Uint16, uint16)
        REFLECTION_ASSIGN_INT_CASE(Uint32, uint32)
        REFLECTION_ASSIGN_INT_CASE(Uint64, uint64)
        #undef REFLECTION_ASSIGN_INT_CASE

        case AssetMetaPropertyType::UnsignedEnum:
        {
          void* toEnumFunctionPointer = findToEnumFunction(reflection.typeName);
          switch(reflection.size)
          {
            #define REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE(size, type) \
              case size: \
              { \
                using ToEnumFunctionPointer = type(*)(const char*); \
                ToEnumFunctionPointer toEnumFunction = (ToEnumFunctionPointer)toEnumFunctionPointer; \
                type value = toEnumFunction(node.value); \
                *(type*)(((byte*)destination) + reflection.offset) = value; \
                break; \
              }
            REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE(1, uint8)
            REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE(2, uint16)
            REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE(4, uint32)
            REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE(8, uint64)
            #undef REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE

            default:
              ensureNoEntry();
              break;
          }
          break;
        }

        case AssetMetaPropertyType::SignedEnum:
        {
          void* toEnumFunctionPointer = findToEnumFunction(reflection.typeName);
          switch(reflection.size)
          {
            #define REFLECTION_ASSIGN_SIGNED_ENUM_CASE(size, type) \
              case size: \
              { \
                using ToEnumFunctionPointer = type(*)(const char*); \
                ToEnumFunctionPointer toEnumFunction = (ToEnumFunctionPointer)toEnumFunctionPointer; \
                type value = toEnumFunction(node.value); \
                *(type*)(((byte*)destination) + reflection.offset) = value; \
                break; \
              }
            REFLECTION_ASSIGN_SIGNED_ENUM_CASE(1, int8)
            REFLECTION_ASSIGN_SIGNED_ENUM_CASE(2, int16)
            REFLECTION_ASSIGN_SIGNED_ENUM_CASE(4, int32)
            REFLECTION_ASSIGN_SIGNED_ENUM_CASE(8, int64)
            #undef REFLECTION_ASSIGN_SIGNED_ENUM_CASE

            default:
              ensureNoEntry();
              break;
          }
          break;
        }

        default:
          ensureNoEntry();
          break;
      }

      return true;
    }
  }

  return false;
}

void defaultInitialiazeMetaProperties(const AssetMetaPropertyReflection* reflections, int64 reflectionCount, void* destination)
{
  TRACE_SCOPE();

  for(int64 propertyIndex = 0; propertyIndex < reflectionCount; ++propertyIndex)
  {
    const AssetMetaPropertyReflection& reflection = reflections[propertyIndex];

    switch(reflection.type)
    {
      #define REFLECTION_ASSIGN_INT_CASE(MetaPropertyType, ValueType) \
        case AssetMetaPropertyType::MetaPropertyType: \
        { \
          ValueType value = (ValueType)reflection.defaultValue; \
          *(ValueType*)(((byte*)destination) + reflection.offset) = value; \
          break; \
        }
      REFLECTION_ASSIGN_INT_CASE(Int8, int8)
      REFLECTION_ASSIGN_INT_CASE(Int16, int16)
      REFLECTION_ASSIGN_INT_CASE(Int32, int32)
      REFLECTION_ASSIGN_INT_CASE(Int64, int64)
      REFLECTION_ASSIGN_INT_CASE(Uint8, uint8)
      REFLECTION_ASSIGN_INT_CASE(Uint16, uint16)
      REFLECTION_ASSIGN_INT_CASE(Uint32, uint32)
      REFLECTION_ASSIGN_INT_CASE(Uint64, uint64)
      #undef REFLECTION_ASSIGN_INT_CASE

      case AssetMetaPropertyType::UnsignedEnum:
      {
        switch(reflection.size)
        {
          #define REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE(size, type) \
            case size: \
            { \
              type value = (type)reflection.defaultValue; \
              *(type*)(((byte*)destination) + reflection.offset) = value; \
              break; \
            }
          REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE(1, uint8)
          REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE(2, uint16)
          REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE(4, uint32)
          REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE(8, uint64)
          #undef REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE

          default:
            ensureNoEntry();
            break;
        }
        break;
      }

      case AssetMetaPropertyType::SignedEnum:
      {
        switch(reflection.size)
        {
          #define REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE(size, type) \
            case size: \
            { \
              type value = (type)reflection.defaultValue; \
              *(type*)(((byte*)destination) + reflection.offset) = value; \
              break; \
            }
          REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE(1, int8)
          REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE(2, int16)
          REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE(4, int32)
          REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE(8, int64)
          #undef REFLECTION_ASSIGN_UNSIGNED_ENUM_CASE

          default:
            ensureNoEntry();
            break;
        }
        break;
      }

      default:
        ensureNoEntry();
        break;
    }
  }
}

class AssetDirectory
{
public:

  std::wstring name;
  std::wstring path;
  std::vector<AssetDirectory> directories;
  std::vector<std::wstring> assetFileNames;
  std::vector<Asset*> assets; // Indices correspond to assetFileNames indices, is nullptr for not loaded assets.

  void loadAssetsIncludingSubdirectories()
  {
    TRACE_SCOPE();

    ensureTrue(isInMainThread());

    for (int32 assetIndex = 0; assetIndex < assets.size(); assetIndex++)
    {
      Asset* asset = assets[assetIndex];
      asset->ref();
    }

    for (AssetDirectory& directory : directories)
    {
      directory.loadAssetsIncludingSubdirectories();
    }
  }
  void unloadAssetsIncludingSubdirectories()
  {
    for (Asset* asset : assets)
    {
      asset->unref();
    }

    for (AssetDirectory& directory : directories)
    {
      directory.unloadAssetsIncludingSubdirectories();
    }
  }
};

AssetDirectory rootDirectory;

static bool tryTraverseDirectory(wchar_t* wildcardPathBuffer, int64 wildcardPathLength, AssetDirectory* parentDirectory, WIN32_FIND_DATA* findData)
{
  HANDLE findHandle;
  
  {
    TRACE_SCOPE("FindFirstFile");
    findHandle = FindFirstFile(wildcardPathBuffer, findData);
    if(!findHandle)
    {
      logError("Couldn't find the assets directory.");
      return false;
    }
  }

  while (FindNextFile(findHandle, findData))
  {
    if (findData->cFileName[0] == L'.')
    {
      continue;
    }
    else if (findData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      TRACE_SCOPE("initializeAssetDirectory");

      // Replace the wildcard with the directory name, followed by a wildcard
      swprintf(wildcardPathBuffer + wildcardPathLength - 1, MAX_PATH, L"%s\\*", findData->cFileName);
      const int64 subdirectoryWildcardPathLength = wildcardPathLength + wcslen(findData->cFileName) + 1;

      AssetDirectory& directory = parentDirectory->directories.emplace_back();
      directory.name = findData->cFileName;
      directory.path = std::wstring(wildcardPathBuffer, wildcardPathBuffer + subdirectoryWildcardPathLength - 2);

      tryTraverseDirectory(wildcardPathBuffer, subdirectoryWildcardPathLength, &directory, findData);
    }
    else
    {
      const wchar_t* fileExtension = getFileExtension(findData->cFileName);
      if (!fileExtension)
      {
        wchar_t filePath[MAX_PATH];
        const int64 pathLength = wildcardPathLength - 1;
        memcpy(filePath, wildcardPathBuffer, sizeof(wchar_t) * pathLength);
        swprintf(filePath + pathLength, arrayLength(filePath), L"%s", findData->cFileName);
        logError("File %S is not a valid asset file.", filePath);
        continue;
      }

      if (wcscmp(fileExtension, L"meta") == 0)
      {
        continue;
      }

      TRACE_SCOPE("initializeAssetMeta");

      wchar_t metaFilePath[MAX_PATH];
      const int64 pathLength = wildcardPathLength - 1;
      memcpy(metaFilePath, wildcardPathBuffer, sizeof(wchar_t) * pathLength);
      const int64 fileNameLength = wcslen(findData->cFileName);
      memcpy(metaFilePath + pathLength, findData->cFileName, sizeof(wchar_t) * fileNameLength);
      metaFilePath[pathLength + fileNameLength] = L'\0';
      wchar_t* metaFilePathExtension = getFileExtension(metaFilePath);
      memcpy(metaFilePathExtension, L"meta\0", sizeof(wchar_t) * 5);
      if (!ensure(fileExists(metaFilePath)))
      {
        swprintf(metaFilePathExtension, MAX_PATH - pathLength - fileNameLength, L"%s", fileExtension);
        logError("File %S doesn't have a corresponding meta file.", metaFilePath);
        continue;
      }

      std::vector<byte> assetMetaFileData;
      ensure(tryReadEntireFile(metaFilePath, assetMetaFileData));
      // Used for second parse as the parse changes the data.
      // TODO: do custom non-intrusive parsing for the assetType to avoid doing this copy.
      std::vector<byte> assetMetaFileDataCopy = assetMetaFileData;

      swprintf(metaFilePathExtension, MAX_PATH - pathLength - fileNameLength, L"%s", fileExtension);
      
      const int64 metaFilePathLength = wcslen(metaFilePath);
      wchar_t* assetPath = new wchar_t[metaFilePathLength + 1];
      swprintf(assetPath, metaFilePathLength + 1, L"%s", metaFilePath);

      parentDirectory->assetFileNames.emplace_back(findData->cFileName);

      AssetType assetType = AssetType::Unknown;
      char* assetMetaData = reinterpret_cast<char*>(assetMetaFileData.data());
      if(!ensure(tryParseConfig(assetMetaData, assetMetaFileData.size(), [&assetType](const ConfigKeyValueNode& node) {
        if(node.isKey("assetType"))
        {
          assetType = assetTypeStringToEnum(node.value);
          return true;
        }

        return false;
        })))
      {
        continue;
      }
      if(!ensure(assetType != AssetType::Unknown))
      {
        continue;
      }

      Asset* assetBase;
      const AssetMetaPropertyReflection* metaPropertyReflections;
      int64 metaPropertyReflectionCount;
      switch(assetType)
      {
        #define ASSET_TYPE_CONSTRUCT(name) \
          case AssetType::name: { \
            TRACE_SCOPE("allocate " #name); \
            name* asset = (name*) malloc(sizeof(name)); \
            parentDirectory->assets.emplace_back(asset); \
            assetBase = asset; \
            assetBase->path = assetPath; \
            assetBase->assetType = assetType; \
            assetBase->refCount = 0; \
            metaPropertyReflections = name::getMetaPropertyReflections(); \
            metaPropertyReflectionCount = name::metaPropertyCount; \
            break; \
          }

        ASSET_TYPE_LIST(ASSET_TYPE_CONSTRUCT)
        #undef ASSET_TYPE_CONSTRUCT

        default:
          ensureNoEntry();
          continue;
      }

      defaultInitialiazeMetaProperties(metaPropertyReflections, metaPropertyReflectionCount, assetBase);

      tryParseConfig((char*)assetMetaFileDataCopy.data(), (int64)assetMetaFileDataCopy.size(), [=](const ConfigKeyValueNode& node) {
        if(parseMetaProperty(metaPropertyReflections, metaPropertyReflectionCount, node, assetBase))
        {
          return false;
        }

        return false;
      });
    }
  }

  FindClose(findHandle);

  return true;
}

bool tryInitializeAssetSystem()
{
  TRACE_SCOPE();

  WIN32_FIND_DATA findData;
  wchar_t wildcardPath[MAX_PATH];
  swprintf(wildcardPath, arrayLength(wildcardPath), L"assets\\*");
  const int64 wildcardPathLength = wcslen(wildcardPath);
  if (!tryTraverseDirectory(wildcardPath, wildcardPathLength, &rootDirectory, &findData))
  {
    logError("Couldn't find the assets directory.");
    return false;
  }

  return true;
}

AssetDirectory* findDirectory(const wchar_t* path)
{
  TRACE_SCOPE();

  ensureTrue(path != nullptr, nullptr);

  const wchar_t* subPath = path;
  AssetDirectory* parentDirectory = &rootDirectory;
  while (true)
  {
    uint64 subPathLength = getLengthWithoutTrailingSlashes(subPath);
    int64 subPathDirectoryNameLength = getLengthUntilFirstSlash(subPath);

    bool nextDirectoryFound = false;
    for (AssetDirectory& directory : parentDirectory->directories)
    {
      if (directory.name.length() == subPathDirectoryNameLength)
      {
        const bool areNamesEqual = contains(directory.name.c_str(), subPath, subPathDirectoryNameLength);
        if (areNamesEqual)
        {
          if (subPathDirectoryNameLength == subPathLength)
          {
            return &directory;
          }

          subPath = subPath + subPathDirectoryNameLength + 1;
          parentDirectory = &directory;
          nextDirectoryFound = true;
          break;
        }
      }
    }

    if (!nextDirectoryFound)
    {
      return nullptr;
    }
  }

  return nullptr;
}

AssetDirectoryRef::AssetDirectoryRef(const wchar_t* path)
{
  initialize(path);
}
AssetDirectoryRef::AssetDirectoryRef(AssetDirectoryRef&& other)
{
  std::swap(directory, other.directory);
}
AssetDirectoryRef::~AssetDirectoryRef()
{
  if(directory)
  {
    directory->unloadAssetsIncludingSubdirectories();
  }
}
void AssetDirectoryRef::initialize(const wchar_t* path)
{
  ensureTrue(isInMainThread());

  if(directory)
  {
    directory->unloadAssetsIncludingSubdirectories();
  }

  directory = findDirectory(path);
  if(ensure(directory))
  {
    directory->loadAssetsIncludingSubdirectories();
  }
}

Asset* internalFindAsset(AssetDirectory* directory, const wchar_t* path)
{
  ensureTrue(directory != nullptr, nullptr);
  ensureTrue(path != nullptr, nullptr);

  AssetDirectory* lastDirectory = directory;
  const wchar_t* subPath = path;
  int64 lengthUntilFirstSlash = getLengthUntilFirstSlash(subPath);
  while (lengthUntilFirstSlash != wcslen(subPath))
  {
    for (AssetDirectory& nextDirectory : directory->directories)
    {
      if (contains(subPath, nextDirectory.name.c_str(), lengthUntilFirstSlash))
      {
        lastDirectory = &nextDirectory;
        subPath += lengthUntilFirstSlash + 1;
        lengthUntilFirstSlash = getLengthUntilFirstSlash(subPath);
        continue;
      }
    }

    ensureNoEntry();
    return nullptr;
  }

  const wchar_t* fileName = subPath;
  for (uint64 assetIndex = 0; assetIndex < lastDirectory->assetFileNames.size(); ++assetIndex)
  {
    const wchar_t* assetFileName = lastDirectory->assetFileNames[assetIndex].c_str();
    if (isEqual(fileName, assetFileName))
    {
      return lastDirectory->assets[assetIndex];
    }
  }

  ensureNoEntry();
  return nullptr;
}

void Config::initialize(const byte* fileData, int64 fileDataLength)
{
  std::vector<char> fileDataCopy;
  fileDataCopy.insert(fileDataCopy.begin(), fileData, fileData + fileDataLength);
  tryParseConfig(fileDataCopy.data(), fileDataLength, [this](const ConfigKeyValueNode& node) -> bool {
    std::string value{node.value, static_cast<uint64>(node.valueLength)};
    std::transform(value.begin(), value.end(), value.begin(), std::tolower);
    keysToValues.emplace(std::string(node.key, node.keyLength), std::move(value));

    return false;
  });
}
bool Config::getBool(const char* key) const
{
  auto it = keysToValues.find(key);
  if (it == keysToValues.end())
  {
    ensureNoEntry();
    return 0;
  }

  const std::string& value = it->second;
  if (value == "true")
  {
    return true;
  }
  else if(value == "false")
  {
    return false;
  }

  char* endPtr;
  int64 number = strtoll(value.c_str(), &endPtr, 10);
  ensureTrue(*endPtr, false);

  if (number != 0)
  {
    return true;
  }
  else 
  {
    return false;
  }
}
float Config::getFloat(const char* key) const
{
  auto it = keysToValues.find(key);
  if (it == keysToValues.end())
  {
    ensureNoEntry();
    return 0;
  }

  return std::stof(it->second);
}
double Config::getDouble(const char* key) const
{
  auto it = keysToValues.find(key);
  if (it == keysToValues.end())
  {
    ensureNoEntry();
    return 0;
  }

  return std::stold(it->second);
}
int64 Config::getInt(const char* key) const
{
  auto it = keysToValues.find(key);
  if (it == keysToValues.end())
  {
    ensureNoEntry();
    return 0;
  }

  return std::stoll(it->second);
}
const std::string& Config::getString(const char* key) const
{
  auto it = keysToValues.find(key);
  ensure(it == keysToValues.end());
  return it->second;
}

DXGI_FORMAT toDxgiFormat(PixelFormat pixelFormat)
{
  switch(pixelFormat)
  {
    case PixelFormat::int16: return DXGI_FORMAT_R16_SINT;
    default: ensureNoEntry(); return DXGI_FORMAT_UNKNOWN;
  }
}

void Texture2D::initialize(const byte* fileData, int64 fileDataLength)
{
  // TODO: Maybe don't use dds files anymore as we have the meta file?

  const wchar_t* fileNameExtension = getFileExtension(path);
  if(isEqual(fileNameExtension, L"dds"))
  {
    if(!ensure(SUCCEEDED(createTextureFromDDS(fileData, fileDataLength, (ID3D11Resource**)&texture, &view))))
    {
      logError("Failed to initialize Texture2D from a dds file.");
      return;
    }

    D3D11_TEXTURE2D_DESC description;
    texture->GetDesc(&description);

    width = description.Width;
    height = description.Height;
    mipLevelCount = description.MipLevels;
    pixelFormat = toPixelFormat(description.Format);

    // TODO: Handle cpu access;
  }
  else
  {
    ensureTrue(width > 0 && height > 0);
    ensureTrue(mipLevelCount > 0);
    ensureTrue(pixelFormat != PixelFormat::Invalid);

    // TODO: Handle cpu access.
    const D3D11_TEXTURE2D_DESC description = {
      UINT(width),
      UINT(height),
      UINT(mipLevelCount),
      1,
      toDxgiFormat(pixelFormat),
      {1, 0},
      D3D11_USAGE_IMMUTABLE,
      D3D11_BIND_SHADER_RESOURCE,
      0,
      0
    };

    D3D11_SUBRESOURCE_DATA subresourceData;
    subresourceData.pSysMem = fileData;
    subresourceData.SysMemPitch = width * toPixelSizeInBytes(pixelFormat);
    subresourceData.SysMemSlicePitch = 0;

    if(FAILED(D3D11::device->CreateTexture2D(&description, &subresourceData, &texture)))
    {
      ensureNoEntry();
      logError("Failed to create heightmap texture.");
      return;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC viewDescription;
    viewDescription.Format = description.Format;
    viewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    viewDescription.Texture2D.MostDetailedMip = 0;
    viewDescription.Texture2D.MipLevels = mipLevelCount;

    if(FAILED(D3D11::device->CreateShaderResourceView(texture, &viewDescription, &view)))
    {
      ensureNoEntry();
      logError("Failed to create heightmap shader resource view.");
      texture.Release();
      return;
    }
  }
}