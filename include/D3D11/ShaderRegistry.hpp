#pragma once

#include "Core/Core.hpp"
#include "Core/File.hpp"
#include "D3D11/D3D11.hpp"

#include <vector>

#include <d3dcompiler.h>

class TaskScheduler;

namespace D3D11
{
  class ShaderRegistry
  {
  public:

    ShaderRegistry() = default;
    ShaderRegistry(const ShaderRegistry& other) = delete;
    ShaderRegistry(ShaderRegistry&& other) = delete;
    ~ShaderRegistry() = default;

    // Recursively searches shaderDirectoryPath for shader files and loads them.
    // Supports both object code and source code files.
    // Previously loaded shaders are discarded.
    void reload(LPCTSTR shaderDirectoryPath);

    ID3D11VertexShader* findVertexShader(LPCTSTR name) const;
    ID3D11PixelShader* findPixelShader(LPCTSTR name) const;

  private:
    
    static constexpr int shaderCountMax = 50;
    static constexpr int shaderNameLengthMax = 32;

    struct VertexShaders
    {
      #define VERTEX_SHADER(name, ...) + 1
      static constexpr int64 count = 0
        #include VERTEX_SHADERS
        ;
      #undef VERTEX_SHADER

      #define VERTEX_SHADER(name, ...) L ## #name ,
      static constexpr const wchar_t* const names[] = {
        #include VERTEX_SHADERS
      };
      #undef VERTEX_SHADER

      #define VERTEX_SHADER(name, ...) nullptr,
      CComPtr<ID3D11VertexShader> handles[count] = {
        #include VERTEX_SHADERS
      };
      #undef VERTEX_SHADER

      #define VERTEX_SHADER(name, ...) nullptr,
      CComPtr<ID3D11InputLayout> inputLayouts[count] = {
        #include VERTEX_SHADERS
      };
      #undef VERTEX_SHADER

      #define VERTEX_SHADER(name, ...) static constexpr D3D11_INPUT_ELEMENT_DESC name ## ElementDescription[] = { __VA_ARGS__ };
        #include VERTEX_SHADERS
      #undef VERTEX_SHADER

      struct InputElementEntry
      {
        const D3D11_INPUT_ELEMENT_DESC* const descriptions;
        int64 count;
      };
      #define VERTEX_SHADER(name, ...) { name ## ElementDescription , arrayLength( name ## ElementDescription )},
      static constexpr InputElementEntry inputElementEntries[count] = {
        #include VERTEX_SHADERS
      };
      #undef VERTEX_SHADER
    };
    VertexShaders vertexShaders;

    struct PixelShaders
    {
      #define PIXEL_SHADER(name) + 1
      static constexpr int64 count = 0
        #include PIXEL_SHADERS
        ;
      #undef PIXEL_SHADER

      #define PIXEL_SHADER(name) L ## #name ,
      static constexpr const wchar_t* const names[] = {
        #include PIXEL_SHADERS
      };
      #undef PIXEL_SHADER

      #define PIXEL_SHADER(name) nullptr,
      CComPtr<ID3D11PixelShader> handles[count] = {
        #include PIXEL_SHADERS
      };
      #undef PIXEL_SHADER
    };
    PixelShaders pixelShaders;

    void reset();

    void load(LPCTSTR shaderDirectoryPath, std::vector<byte>& shaderBytecode);

    void loadVertexShaderSourceFile(LPCTSTR filePath, LPCTSTR fileName);
    void loadVertexShaderObjectFile(LPCTSTR filePath, LPCTSTR fileName, std::vector<byte>& shaderBytecode);
    void createVertexShader(LPCTSTR fileName, const void* shaderBytecode, uint64 shaderBytecodeSize);

    void loadPixelShaderSourceFile(LPCTSTR filePath, LPCTSTR fileName);
    void loadPixelShaderObjectFile(LPCTSTR filePath, LPCTSTR fileName, std::vector<byte>& shaderBytecode);
    void createPixelShader(LPCTSTR fileName, const void* shaderBytecode, uint64 shaderBytecodeSize);

  };

  // IMPLEMENTATION DETAILS BELOW

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
    for (int64 i = 0; i < vertexShaders.count; i++)
    {
      vertexShaders.handles[i] = nullptr;
      vertexShaders.inputLayouts[i] = nullptr;
    }

    for (int64 i = 0; i < pixelShaders.count; i++)
    {
      pixelShaders.handles[i] = nullptr;
    }
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

  template <typename ShadersType>
  int64 findShader(LPCTSTR shaderFileName, ShadersType& shaders)
  {
    for (int64 shaderIndex = 0; shaderIndex < shaders.count; ++shaderIndex)
    {
      LPCTSTR shaderName = shaders.names[shaderIndex];
      int64 shaderFileNameCharOffset = 0;

      while (*shaderName != L'\0')
      {
        if (*shaderName != *(shaderFileName + shaderFileNameCharOffset++))
        {
          break;
        }
        shaderName++;
      }

      if (*shaderName == L'\0' && *(shaderFileName + shaderFileNameCharOffset) == L'.')
      {
        return shaderIndex;
      }
    }

    return -1;
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
    const int64 shaderIndex = findShader(fileName, vertexShaders);
    if (shaderIndex < 0)
    {
      logError("There is no vertex shader %S", fileName);
      return;
    }

    if (FAILED(device->CreateVertexShader(shaderBytecode, shaderBytecodeSize, nullptr, &vertexShaders.handles[shaderIndex])))
    {
      logError("Failed to create vertex shader %S", fileName);
      return;
    }

    const D3D11_INPUT_ELEMENT_DESC* inputElementDescriptions = vertexShaders.inputElementEntries[shaderIndex].descriptions;
    int64 inputElementDescriptionCount = vertexShaders.inputElementEntries[shaderIndex].count;

    if (inputElementDescriptions == nullptr)
    {
      logError("Failed to get input element description for vertex shader %S", fileName);
      return;
    }

    if (FAILED(device->CreateInputLayout(inputElementDescriptions, static_cast<UINT>(inputElementDescriptionCount), shaderBytecode, shaderBytecodeSize, &vertexShaders.inputLayouts[shaderIndex]))) {
      logError("Failed to create input layout for vertex shader %S", fileName);
      return;
    }

    logInfo("Successfully loaded vertex shader %S", fileName);
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
    const int64 shaderIndex = findShader(fileName, pixelShaders);
    if (shaderIndex < 0)
    {
      logError("There is no pixel shader %S", fileName);
      return;
    }

    if (FAILED(device->CreatePixelShader(shaderBytecode, shaderBytecodeSize, nullptr, &pixelShaders.handles[shaderIndex])))
    {
      logError("Failed to create pixel shader %S", fileName);
      return;
    }

    logInfo("Successfully loaded pixel shader %S", fileName);
  }

}