#define DAR_MODULE_NAME "Asset"

#include "Core/Asset.hpp"

#include <string>

#include "Core/File.hpp"
#include "Core/String.hpp"
#include "Core/Config.hpp"

#define FIND_ASSET_IMPLEMENTATION(name) \
  template<> \
  name* AssetDirectoryRef::findAsset<name>(const wchar_t* path) const \
  { \
    Asset* asset = internalFindAsset(directory, path); \
    if(!ensure(asset != nullptr)) return nullptr; \
    if(!ensure(asset->assetType == AssetType::name)) return nullptr; \
    return reinterpret_cast<name*>(asset); \
  }
ASSET_TYPE_LIST(FIND_ASSET_IMPLEMENTATION)

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

static wchar_t* getFileExtension(wchar_t* fileName)
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

void Asset::ref()
{
  refCount++;
}

void Asset::unref()
{
  if (--refCount == 0)
  {
    ensureTrue(isInMainThread());

    *pointerInAssetDirectory = nullptr;

    switch (assetType)
    {
    #define ASSET_TYPE_DELETE_CASE(name) case AssetType::name: {\
      name* derivedPtr = reinterpret_cast<name*>(this); \
      delete derivedPtr; \
      break; }
    
      ASSET_TYPE_LIST(ASSET_TYPE_DELETE_CASE)
    #undef ASSET_TYPE_DELETE_CASE

      default:
        ensureNoEntry();
        break;
    }
  }
}

struct AssetDirectory
{
  std::wstring name;
  std::wstring path;
  std::vector<AssetDirectory> directories;
  std::vector<std::wstring> assetFileNames;
  std::vector<Asset*> assets; // Indices correspond to assetFileNames indices, is nullptr for not loaded assets.

  void loadAssetsIncludingSubdirectories()
  {
    ensureTrue(isInMainThread());

    for (int32 assetIndex = 0; assetIndex < assets.size(); assetIndex++)
    {
      Asset* asset = assets[assetIndex];
      if (asset)
      {
        asset->ref();
        continue;
      }

      // TODO: asynchronize the loading and initialization

      wchar_t assetPath[MAX_PATH];
      swprintf(assetPath, arrayLength(assetPath), L"%s\\%s", path.c_str(), assetFileNames[assetIndex].c_str());
      std::vector<byte> assetFileData;
      ensure(tryReadEntireFile(assetPath, assetFileData));

      wchar_t* fileExtension = getFileExtension(assetPath);
      memcpy(fileExtension, L"meta\0", sizeof(wchar_t) * 5);
      std::vector<byte> assetMetaFileData;
      ensure(tryReadEntireFile(assetPath, assetMetaFileData));

      AssetType assetType = AssetType::Unknown;
      char* assetMetaData = reinterpret_cast<char*>(assetMetaFileData.data());
      if (!ensure(tryParseConfig(assetMetaData, assetMetaFileData.size(), [&assetType](const ConfigKeyValueNode& node) -> bool {
          if (node.isKey("AssetType"))
          {
            assetType = assetTypeStringToEnum(node.value);
            return true;
          }

          return false;
        })))
      {
        continue;
      }
      if (!ensure(assetType != AssetType::Unknown))
      {
        continue;
      }

      Asset** pointerInAssetDirectory = &assets[assetIndex];
      Asset* assetBase = nullptr;

      switch (assetType)
      {
      #define ASSET_TYPE_CONSTRUCT(name) \
        case AssetType::name: { \
          name* asset = new name(); \
          asset->initialize(assetMetaFileData.data(), assetMetaFileData.size(), assetFileData.data(), assetFileData.size()); \
          assetBase = asset; \
            break; }

      ASSET_TYPE_LIST(ASSET_TYPE_CONSTRUCT)
      #undef ASSET_TYPE_CONSTRUCT

        default:
          ensureNoEntry();
          continue;
      }

      assetBase->ref();
      *pointerInAssetDirectory = assetBase;

      assetBase->pointerInAssetDirectory = pointerInAssetDirectory;
      assetBase->assetType = assetType;
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
  HANDLE findHandle = FindFirstFile(wildcardPathBuffer, findData);
  if (!findHandle)
  {
    logError("Couldn't find the assets directory.");
    return false;
  }

  while (FindNextFile(findHandle, findData))
  {
    if (findData->cFileName[0] == L'.')
    {
      continue;
    }
    else if (findData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
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
        const int64 fileExtensionLength = static_cast<int64>(wcslen(fileExtension));
        memcpy(metaFilePathExtension, fileExtension, fileExtensionLength);
        metaFilePathExtension[fileExtensionLength] = L'0';
        swprintf(metaFilePathExtension, MAX_PATH - pathLength - fileNameLength, L"%s", fileExtension);
        logError("File %S doesn't have a corresponding meta file.", metaFilePath);
        continue;
      }

      parentDirectory->assetFileNames.emplace_back(findData->cFileName);
    }
  }

  FindClose(findHandle);

  parentDirectory->assets.resize(parentDirectory->assetFileNames.size(), nullptr);

  return true;
}

bool tryInitializeAssetSystem()
{
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
  : directory(findDirectory(path))
{
  ensureTrue(isInMainThread());
  ensureTrue(directory != nullptr);

  directory->loadAssetsIncludingSubdirectories();
  // TODO: Traverse the subtree, increase ref count and add each assets loaded task event to an array, 
  // which will be a prerequisite to a task that sets the returned task event ref to completed.
}
AssetDirectoryRef::~AssetDirectoryRef()
{
  directory->unloadAssetsIncludingSubdirectories();
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
  for (int64 assetIndex = 0; assetIndex < lastDirectory->assetFileNames.size(); ++assetIndex)
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

void Config::initialize(byte* metaData, int64 metaDataLength, byte* fileData, int64 fileDataLength)
{
  tryParseConfig(reinterpret_cast<char*>(fileData), fileDataLength, [this](const ConfigKeyValueNode& node) -> bool {
    std::string value{node.value, static_cast<uint64>(node.valueLength)};
    std::transform(value.begin(), value.end(), value.begin(), std::tolower);
    keysToValues.emplace(std::string(node.key, node.keyLength), std::move(value));

      return false;
  });
}
bool Config::getBool(const std::string& key) const
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
float Config::getFloat(const std::string& key) const
{
  auto it = keysToValues.find(key);
  if (it == keysToValues.end())
  {
    ensureNoEntry();
    return 0;
  }

  return std::stof(it->second);
}
double Config::getDouble(const std::string& key) const
{
  auto it = keysToValues.find(key);
  if (it == keysToValues.end())
  {
    ensureNoEntry();
    return 0;
  }

  return std::stold(it->second);
}
int64 Config::getInt(const std::string& key) const
{
  auto it = keysToValues.find(key);
  if (it == keysToValues.end())
  {
    ensureNoEntry();
    return 0;
  }

  return std::stoll(it->second);
}
std::string Config::getString(const std::string& key) const
{
  auto it = keysToValues.find(key);
  if (it == keysToValues.end())
  {
    ensureNoEntry();
    return {};
  }

  return it->second;
}

void Texture2D::initialize(byte* metaData, int64 metaDataLength, byte* fileData, int64 fileDataLength)
{
  // TODO: implement
}