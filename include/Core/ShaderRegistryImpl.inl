#include "Core/Core.hpp"
#include "Core/File.hpp"
#include "Core/String.hpp"
#include "Core/D3D11.hpp"

#include <vector>

namespace D3D11
{
  #define VERTEX_SHADER(name, ...) CComPtr<ID3D11VertexShader> name ## VertexShader ;
  #include VERTEX_SHADERS
  #undef VERTEX_SHADER
  #define VERTEX_SHADER(name, ...) CComPtr<ID3D11InputLayout> name ## InputLayout ;
  #include VERTEX_SHADERS
  #undef VERTEX_SHADER
  #define VERTEX_SHADER(name, ...) constexpr D3D11_INPUT_ELEMENT_DESC name ## InputElementDescriptions[] = { __VA_ARGS__ };
  #include VERTEX_SHADERS
  #undef VERTEX_SHADER
  #define PIXEL_SHADER(name) CComPtr<ID3D11PixelShader> name ## PixelShader ;
  #include PIXEL_SHADERS
  #undef PIXEL_SHADER

  void loadVertexShader(LPCTSTR shaderName, LPCTSTR shaderDirectoryPath, std::vector<byte>& shaderBytecode,
    const D3D11_INPUT_ELEMENT_DESC* inputElementDescriptions, uint64 inputElementDescriptionCount, CComPtr<ID3D11VertexShader>& outVertexShader, CComPtr<ID3D11InputLayout>& outInputLayout)
  {
    TCHAR filePath[128];
    int64 filePathIndex = combine(shaderDirectoryPath, shaderName, filePath);
    filePath[filePathIndex++] = L'.';
    filePath[filePathIndex++] = L'v';
    filePath[filePathIndex++] = L's';
    filePath[filePathIndex++] = L'.';
    filePath[filePathIndex++] = L'c';
    filePath[filePathIndex++] = L's';
    filePath[filePathIndex++] = L'o';
    filePath[filePathIndex] = L'\0';

    if (!tryReadEntireFile(filePath, shaderBytecode))
    {
      logError("Failed to load vertex shader object file %S", filePath);
      return;
    }

    if (FAILED(device->CreateVertexShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &outVertexShader)))
    {
      logError("Failed to create vertex shader %S", shaderName);
      return;
    }

    if (FAILED(device->CreateInputLayout(inputElementDescriptions, static_cast<UINT>(inputElementDescriptionCount), shaderBytecode.data(), shaderBytecode.size(), &outInputLayout))) {
      logError("Failed to create input layout for vertex shader %S", shaderName);
      return;
    }
  }

  void loadPixelShader(LPCTSTR shaderName, LPCTSTR shaderDirectoryPath, std::vector<byte>& shaderBytecode, CComPtr<ID3D11PixelShader>& outPixelShader)
  {
    TCHAR filePath[128];
    int64 filePathIndex = combine(shaderDirectoryPath, shaderName, filePath);
    filePath[filePathIndex++] = L'.';
    filePath[filePathIndex++] = L'p';
    filePath[filePathIndex++] = L's';
    filePath[filePathIndex++] = L'.';
    filePath[filePathIndex++] = L'c';
    filePath[filePathIndex++] = L's';
    filePath[filePathIndex++] = L'o';
    filePath[filePathIndex] = L'\0';

    if (!tryReadEntireFile(filePath, shaderBytecode))
    {
      logError("Failed to load pixel shader object file %S", filePath);
      return;
    }

    if (FAILED(device->CreatePixelShader(shaderBytecode.data(), shaderBytecode.size(), nullptr, &outPixelShader)))
    {
      logError("Failed to create pixel shader %S", shaderName);
      return;
    }
  }

  void resetShaders()
  {
    #define VERTEX_SHADER(name, ...) name ## VertexShader = nullptr; name ## InputLayout = nullptr;
    #include VERTEX_SHADERS
    #undef VERTEX_SHADER
    #define PIXEL_SHADER(name) name ## PixelShader = nullptr;
    #include PIXEL_SHADERS
    #undef PIXEL_SHADER
  }

  void reloadShaders(LPCTSTR shaderDirectoryPath)
  {
    resetShaders();

    std::vector<byte> shaderBytecode;

    #define VERTEX_SHADER(name, ...) loadVertexShader(L ## #name, shaderDirectoryPath, shaderBytecode, name ## InputElementDescriptions, arrayLength(name ## InputElementDescriptions), name ## VertexShader, name ## InputLayout);
    #include VERTEX_SHADERS
    #undef VERTEX_SHADER
    #define PIXEL_SHADER(name) loadPixelShader(L ## #name, shaderDirectoryPath, shaderBytecode, name ## PixelShader);
    #include PIXEL_SHADERS
    #undef PIXEL_SHADER

    logInfo("Shaders reloaded.");
  }
}