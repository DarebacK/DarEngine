#define DAR_MODULE_NAME "Asset"

#include "Core/Asset.hpp"

#include "Core/File.hpp"

struct AssetDirectory
{
  std::wstring name;
  std::vector<AssetDirectory> directories;
  std::vector<std::wstring> assetFileNames;
};

AssetDirectory rootDirectory;

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
        wchar_t filePath[260];
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

      //TODO: check if meta file exists for this asset file

      parentDirectory->assetFileNames.emplace_back(findData->cFileName);
    }
  }

  FindClose(findHandle);

  return true;
}

bool tryInitializeAssetSystem()
{
  // TODO: traverse the assets directory and create the AssetDirectory tree.
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

AssetDirectoryRef::~AssetDirectoryRef()
{
  // TODO: Traverse the subtree, decrease ref count.
}
AssetDirectoryRef loadAssetDirectory(const wchar_t* path)
{
  // TODO: Traverse the subtree, increase ref count and add each assets loaded task event to an array, 
  // which will be a prerequisite to a task that sets the returned task event ref to completed.

  return AssetDirectoryRef(nullptr);
}