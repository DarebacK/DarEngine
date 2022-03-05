#define DAR_MODULE_NAME "D3D11"

#include "D3D11/ShaderRegistry.hpp"

#include "Core/File.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

#include <d3dcompiler.h>

namespace D3D11
{

  constexpr UINT compileFlags = 0
#ifdef DAR_DEBUG
  | D3DCOMPILE_DEBUG
  | D3DCOMPILE_SKIP_OPTIMIZATION
#else
  | D3DCOMPILE_OPTIMIZATION_LEVEL3
#endif
  | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR
  ;

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

  void createShaderFilePath(LPCTSTR shaderDirectoryPath, TCHAR* shaderFileName, TCHAR* destination)
  {
    int64 directoryPathLength = 0;
    while (*(shaderDirectoryPath + directoryPathLength) != L'\0')
    {
      destination[directoryPathLength++] = *(shaderDirectoryPath + directoryPathLength);
    }
    _tcscpy_s(destination + directoryPathLength - 1, 256 - directoryPathLength + 1, shaderFileName); // -1/+1 because of cutting of the wildcard star.
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
                      createShaderFilePath(shaderDirectoryPath, fileData.cFileName, filePath);
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
                      TCHAR filePath[256];
                      createShaderFilePath(shaderDirectoryPath, fileData.cFileName, filePath);
                      loadVertexShaderSourceFile(filePath, fileData.cFileName);
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
                      TCHAR filePath[256];
                      createShaderFilePath(shaderDirectoryPath, fileData.cFileName, filePath);
                      loadPixelShaderObjectFile(filePath, fileData.cFileName, shaderBytecode);
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
                      TCHAR filePath[256];
                      createShaderFilePath(shaderDirectoryPath, fileData.cFileName, filePath);
                      loadPixelShaderSourceFile(filePath, fileData.cFileName);
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

  void ShaderRegistry::loadVertexShaderSourceFile(LPCTSTR filePath, LPCTSTR fileName)
  {
    CComPtr<ID3DBlob> compiledCode;
    if (FAILED(D3DCompileFromFile(filePath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "vertexShaderMain", "vs_5_0", compileFlags, 0, &compiledCode, nullptr)))
    {
      logError("Failed to compile vertex shader %S", filePath);
      return;
    }

    createVertexShader(fileName, compiledCode->GetBufferPointer(), compiledCode->GetBufferSize());
  }

  void ShaderRegistry::loadVertexShaderObjectFile(LPCTSTR filePath, LPCTSTR fileName, std::vector<byte>& shaderBytecode)
  {
    if (!tryReadEntireFile(filePath, shaderBytecode))
    {
      logError("Failed to load vertex shader object file %S", filePath);
      return;
    }

    createVertexShader(fileName, shaderBytecode.data(), shaderBytecode.size());
  }

  void ShaderRegistry::createVertexShader(LPCTSTR fileName, const void* shaderBytecode, uint64 shaderBytecodeSize)
  {
    if (FAILED(device->CreateVertexShader(shaderBytecode, shaderBytecodeSize, nullptr, &vertexShaders.handles[vertexShaders.count])))
    {
      logError("Failed to create vertex shader %S", fileName);
      return;
    }

    vertexShaders.addName(fileName);
    const wchar_t* shaderName = vertexShaders.names[vertexShaders.count];

    const D3D11_INPUT_ELEMENT_DESC* inputElementDescriptions;
    int64 inputElementDescriptionCount;
    getInputElementDescriptions(shaderName, &inputElementDescriptions, &inputElementDescriptionCount);

    if (inputElementDescriptions == nullptr)
    {
      logError("Failed to get input element description for vertex shader %S", shaderName);
      return;
    }

    if (FAILED(device->CreateInputLayout(inputElementDescriptions, static_cast<UINT>(inputElementDescriptionCount), shaderBytecode, shaderBytecodeSize, &vertexShaders.inputLayouts[vertexShaders.count]))) {
      logError("Failed to create input layout for vertex shader %S", shaderName);
      return;
    }

    vertexShaders.count++;
  }

  void ShaderRegistry::loadPixelShaderSourceFile(LPCTSTR filePath, LPCTSTR fileName)
  {
    CComPtr<ID3DBlob> compiledCode;
    if (FAILED(D3DCompileFromFile(filePath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "pixelShaderMain", "ps_5_0", compileFlags, 0, &compiledCode, nullptr)))
    {
      logError("Failed to compile pixel shader %S", filePath);
      return;
    }

    createPixelShader(fileName, compiledCode->GetBufferPointer(), compiledCode->GetBufferSize());
  }

  void ShaderRegistry::loadPixelShaderObjectFile(LPCTSTR filePath, LPCTSTR fileName, std::vector<byte>& shaderBytecode)
  {
    if (!tryReadEntireFile(filePath, shaderBytecode))
    {
      logError("Failed to load pixel shader object file %S", filePath);
      return;
    }

    createPixelShader(fileName, shaderBytecode.data(), shaderBytecode.size());
  }

  void ShaderRegistry::createPixelShader(LPCTSTR fileName, const void* shaderBytecode, uint64 shaderBytecodeSize)
  {
    if (FAILED(device->CreatePixelShader(shaderBytecode, shaderBytecodeSize, nullptr, &pixelShaders.handles[pixelShaders.count])))
    {
      logError("Failed to create pixel shader %S", fileName);
      return;
    }

    pixelShaders.addName(fileName);

    pixelShaders.count++;
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

  template <typename ShaderType>
  void ShaderRegistry::Shaders<ShaderType>::addName(LPCTSTR fileName)
  {
    int64 shaderNameLength = 0;
    while (*fileName != L'\0' && *fileName != L'.') // Cut off name extensions.
    {
      namesBuffer[namesBufferEnd + shaderNameLength++] = *fileName;
      fileName++;
    }
    namesBuffer[namesBufferEnd + shaderNameLength] = L'\0';
    const wchar_t* shaderName = namesBuffer + namesBufferEnd;
    names[count] = shaderName;
    namesBufferEnd += shaderNameLength + 1;
    assert(namesBufferEnd <= arrayLength(namesBuffer) + 1);
  }

};

