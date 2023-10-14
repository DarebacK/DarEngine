#define DAR_MODULE_NAME "Asset"

#include "Core/Asset.hpp"

#include "Core/File.hpp"

struct AssetDirectory
{
  std::wstring name;
  std::vector<AssetDirectory> directories;
  std::vector<Asset*> assets;
};

AssetDirectory rootDirectory;

bool tryTraverseDirectory(wchar_t* wildcardPathBuffer, int64 wildcardPathLength, WIN32_FIND_DATA* findData)
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
      logInfo("Traversing directory %S", findData->cFileName);
      // Replace the wildcard with the directory name, followed by a wildcard
      swprintf(wildcardPathBuffer + wildcardPathLength - 1, MAX_PATH, L"%s\\*", findData->cFileName);
      const int64 subdirectoryWildcardPathLength = wildcardPathLength + wcslen(findData->cFileName) + 1;

      // TODO: create asset directory

      tryTraverseDirectory(wildcardPathBuffer, subdirectoryWildcardPathLength, findData);
    }
    else
    {
      // TODO: check whether it is the .meta file. If yes then create the Asset.

      wchar_t filePath[260];
      const int64 pathLength = wildcardPathLength - 1;
      memcpy(filePath, wildcardPathBuffer, sizeof(wchar_t) * pathLength);
      swprintf(filePath + pathLength, arrayLength(filePath), L"%s", findData->cFileName);

      logInfo("Traversing file %S", filePath);
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
  if (!tryTraverseDirectory(wildcardPath, wildcardPathLength, &findData))
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