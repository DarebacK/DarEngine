#include "Core/Asset.hpp"

#include "Core/File.hpp"

struct AssetDirectory
{
  std::wstring name;
  std::vector<AssetDirectory> directories;
  std::vector<Asset*> assets;
};

AssetDirectory rootDirectory;

void initializeAssetSystem()
{
  // TODO: traverse the assets directory and create the AssetDirectory tree.
  //FindFirstFile()
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