#define DAR_MODULE_NAME "Asset"

#include "Core/Asset.hpp"

#include "Core/File.hpp"

struct AssetDirectory
{
  std::wstring name;
  std::vector<AssetDirectory> directories;
  std::vector<std::wstring> assetFileNames;
  std::vector<Asset*> assets; // Indices correspond to assetFileNames indices, is nullptr for not loaded assets.
};

AssetDirectory rootDirectory;

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
    // Ignore trailing slashes
    uint64 subPathLength = wcslen(subPath);
    while (subPath[subPathLength - 1] == L'\\' || subPath[subPathLength - 1] == L'/')
    {
      subPathLength--;
    }

    int64 subPathDirectoryNameLength = 0;
    while (subPath[subPathDirectoryNameLength] != L'\\' && subPath[subPathDirectoryNameLength] != L'/' && subPath[subPathDirectoryNameLength] != L'\0')
    {
      subPathDirectoryNameLength++;
    }

    bool nextDirectoryFound = false;
    for (AssetDirectory& directory : parentDirectory->directories)
    {
      if (directory.name.length() == subPathDirectoryNameLength)
      {
        bool areNamesEqual = true;
        for (int64 i = 0; i < subPathDirectoryNameLength; ++i)
        {
          if (directory.name[i] != subPath[i])
          {
            areNamesEqual = false;
            break;
          }
        }

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
  ensureTrue(directory != nullptr);


  // TODO: Traverse the subtree, increase ref count and add each assets loaded task event to an array, 
  // which will be a prerequisite to a task that sets the returned task event ref to completed.
}
AssetDirectoryRef::~AssetDirectoryRef()
{
  // TODO: Traverse the subtree, decrease ref count.
}
