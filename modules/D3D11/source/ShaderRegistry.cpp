#define DAR_MODULE_NAME "D3D11"

#include "D3D11/ShaderRegistry.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

namespace D3D11
{
  ShaderRegistry::ShaderRegistry()
  {
    reset();
  }

  void ShaderRegistry::reload(LPCTSTR shaderDirectoryPath, TaskScheduler* taskScheduler)
  {
    TCHAR searchQuery[256];
    _tcscpy(searchQuery, shaderDirectoryPath);

    const size_t shaderDirectoryPathLength = _tcslen(shaderDirectoryPath);
    searchQuery[shaderDirectoryPathLength] = L'*';
    searchQuery[shaderDirectoryPathLength + 1] = L'\0';

    load(searchQuery, taskScheduler);
  }

  void ShaderRegistry::reset()
  {
    for (int64 i = 0; i < shaderCountMax; ++i)
    {
      vertexShaderNames[i] = nullptr;
      vertexShaders[i] = nullptr;
      vertexShaderCount = 0;

      pixelShaderNames[i] = nullptr;
      pixelShaders[i] = nullptr;
      pixelShaderCount = 0;
    }
  }

  void ShaderRegistry::load(LPCTSTR shaderDirectoryPath, TaskScheduler* taskScheduler)
  {
    WIN32_FIND_DATA fileData;
    void* searchHandle = FindFirstFile(shaderDirectoryPath, &fileData);
    if (searchHandle == INVALID_HANDLE_VALUE)
    {
      logError("Failed to reload shaders. Couldn't find first file in %S.", shaderDirectoryPath);
      return;
    }

    do
    {
      if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        load(fileData.cFileName, taskScheduler);
      }
      else
      {

      }
    } while (FindNextFile(searchHandle, &fileData));
  }
};