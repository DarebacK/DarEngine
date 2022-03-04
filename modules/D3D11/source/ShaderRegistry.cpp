#define DAR_MODULE_NAME "D3D11"

#include "D3D11/ShaderRegistry.hpp"

#include "Core/File.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

namespace D3D11
{

  void ShaderRegistry::reload(LPCTSTR shaderDirectoryPath)
  {
    reset();

    TCHAR searchQuery[256];
    _tcscpy_s(searchQuery, shaderDirectoryPath);

    size_t searchQueryPathLength = _tcslen(shaderDirectoryPath);
    searchQuery[searchQueryPathLength++] = L'\\';
    searchQuery[searchQueryPathLength++] = L'*';
    searchQuery[searchQueryPathLength++] = L'\0';

    std::vector<byte> shaderBytecode;
    load(searchQuery, shaderBytecode);
  }

  void ShaderRegistry::reset()
  {
    vertexShaders.reset();
    pixelShaders.reset();
  }

#define isChar(x) (fileData.cFileName[charIndex] == x)
#define isFileNameEnd() (fileData.cFileName[charIndex] == L'\0')
#define logInvalidFile() logError("Invalid file in shader directory: %S", fileData.cFileName)

  void ShaderRegistry::load(LPCTSTR shaderDirectoryPath, std::vector<byte>& shaderBytecode)
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

          load(searchQuery, shaderBytecode);
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
                      TCHAR filePath[256];
                      int64 directoryPathLength = 0;
                      while (*(shaderDirectoryPath + directoryPathLength) != L'\0')
                      {
                        filePath[directoryPathLength++] = *(shaderDirectoryPath + directoryPathLength);
                      }
                      _tcscpy_s(filePath + directoryPathLength - 1, arrayLength(filePath) - directoryPathLength + 1, fileData.cFileName); // -1/+1 because of the wildcard star.

                      loadVertexShaderObjectFile(filePath, fileData.cFileName, shaderBytecode);
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
              charIndex++;
              if (isChar(L's'))
              {
                charIndex++;
                if (isChar(L'.'))
                {
                  charIndex++;
                  const size_t remainingCharacterLength = fileNameLength - charIndex;
                  if (remainingCharacterLength == 3)
                  {
                    if (fileData.cFileName[charIndex] == L'c' && fileData.cFileName[charIndex + 1] == L's' && fileData.cFileName[charIndex + 2] == L'o')
                    {
                      logInfo("Pixel shader cso file detected: %S", fileData.cFileName);
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
                      logInfo("Pixel shader source file detected: %S", fileData.cFileName);
                    }
                    else
                    {
                      logInvalidFile();
                      break;
                    }
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

  void ShaderRegistry::loadVertexShaderObjectFile(LPCTSTR filePath, LPCTSTR fileName, std::vector<byte>& shaderBytecode)
  {
    if (!tryReadEntireFile(filePath, shaderBytecode))
    {
      logError("Failed to load vertex shader object file %S", filePath);
      return;
    }

    if (FAILED(device->CreateVertexShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &vertexShaders.handles[vertexShaders.count])))
    {
      logError("Failed to create vertex shader %S", filePath);
      return;
    }

    int64 shaderNameLength = 0;
    while (*fileName != L'\0' && *fileName != L'.')
    {
      vertexShaders.namesBuffer[vertexShaders.namesBufferEnd + shaderNameLength++] = *fileName;
      fileName++;
    }
    vertexShaders.namesBuffer[vertexShaders.namesBufferEnd + shaderNameLength] = L'\0';
    const wchar_t* shaderName = vertexShaders.namesBuffer + vertexShaders.namesBufferEnd;
    vertexShaders.names[vertexShaders.count] = shaderName;
    vertexShaders.namesBufferEnd += shaderNameLength + 1;

    const D3D11_INPUT_ELEMENT_DESC* inputElementDescriptions;
    int64 inputElementDescriptionCount;
    getInputElementDescriptions(shaderName, &inputElementDescriptions, &inputElementDescriptionCount);

    if (inputElementDescriptions == nullptr)
    {
      logError("Failed to get input element description for vertex shader %S", shaderName);
      return;
    }

    if (FAILED(device->CreateInputLayout(inputElementDescriptions, inputElementDescriptionCount, shaderBytecode.data(), shaderBytecode.size(), &vertexShaders.inputLayouts[vertexShaders.count]))) {
      logError("Failed to create input layout for vertex shader %S", shaderName);
      return;
    }

    vertexShaders.count++;
  }

  template<typename ShaderType>
  void ShaderRegistry::Shaders<ShaderType>::reset()
  {
    for (int64 i = 0; i < shaderCountMax; ++i)
    {
      handles[i] = nullptr;
      names[i] = nullptr;
      namesBufferEnd = 0;
      count = 0;
    }
  }

};

