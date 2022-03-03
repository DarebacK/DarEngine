#define DAR_MODULE_NAME "D3D11"

#include "D3D11/ShaderRegistry.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

namespace D3D11
{

  void ShaderRegistry::reload(LPCTSTR shaderDirectoryPath, TaskScheduler& taskScheduler)
  {
    reset();

    TCHAR searchQuery[256];
    _tcscpy_s(searchQuery, shaderDirectoryPath);

    size_t searchQueryPathLength = _tcslen(shaderDirectoryPath);
    searchQuery[searchQueryPathLength++] = L'\\';
    searchQuery[searchQueryPathLength++] = L'*';
    searchQuery[searchQueryPathLength++] = L'\0';

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

#define isChar(x) (fileData.cFileName[charIndex] == x)
#define isFileNameEnd() (fileData.cFileName[charIndex] == L'\0')
#define logInvalidFile() logError("Invalid file in shader directory: %S", fileData.cFileName)

  void ShaderRegistry::load(LPCTSTR shaderDirectoryPath, TaskScheduler& taskScheduler)
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
      int32 charIndex = 0;

      if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        if (!isFileNameEnd() && !isChar(L'.'))
        {
          TCHAR searchQuery[256];
          wcscpy_s(searchQuery, shaderDirectoryPath);

          searchQuery[_tcslen(searchQuery) - 1] = L'\0'; // Remove the star from wildcard

          _tcscat_s(searchQuery, fileData.cFileName);

          size_t searchQueryLength = _tcslen(searchQuery);
          searchQuery[searchQueryLength++] = L'\\';
          searchQuery[searchQueryLength++] = L'*';
          searchQuery[searchQueryLength] = L'\0';

          load(searchQuery, taskScheduler);
        }
      }
      else
      {
        const size_t fileNameLength = _tcsclen(fileData.cFileName);

        while (true)
        {
          if (isChar(L'.'))
          {
            charIndex++;
            if (isChar(L'v'))
            {
              charIndex++;
              if (isChar(L's'))
              {
                charIndex++;
                if(isChar(L'.'))
                {
                  charIndex++;
                  const size_t remainingCharacterLength = fileNameLength - charIndex;
                  if (remainingCharacterLength == 3)
                  {
                    if (fileData.cFileName[charIndex] == L'c' && fileData.cFileName[charIndex + 1] == L's' && fileData.cFileName[charIndex + 2] == L'o')
                    {
                      logInfo("Vertex shader cso file detected: %S", fileData.cFileName);
                      break;
                    }
                    else
                    {
                      logInvalidFile();
                      break;
                    }
                  }
                  else if (remainingCharacterLength == 4)
                  {
                    if (fileData.cFileName[charIndex] == L'h' && fileData.cFileName[charIndex + 1] == L'l' && fileData.cFileName[charIndex + 2] == L's' && fileData.cFileName[charIndex + 3] == L'l')
                    {
                      logInfo("Vertex shader source file detected: %S", fileData.cFileName);
                    }
                    else
                    {
                      logInvalidFile();
                      break;
                    }
                  }
                  else
                  {
                    logInvalidFile();
                    break;
                  }
                }
                else
                {
                  logInvalidFile();
                  break;
                }
              }
              else 
              {
                logInvalidFile();
                break;
              }
            }
            else if (isChar(L'p'))
            {

            }
          }
          else if (isFileNameEnd())
          {
            logInvalidFile();
            break;
          }
          else
          {
            charIndex++;
          }
        }
      }
    } while (FindNextFile(searchHandle, &fileData));
  }
};