#pragma once

#include "Core/Core.hpp"
#include "D3D11/D3D11.hpp"

#include <vector>

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

  protected:

    // TODO: replace with DEFINE_INPUT_ELEMENT_DESCRIPTIONS macro?
    virtual void getInputElementDescriptions(const wchar_t* vertexShaderName, const D3D11_INPUT_ELEMENT_DESC** descriptions, int64* count) = 0;

  private:
    
    static constexpr int shaderCountMax = 50;
    static constexpr int shaderNameLengthMax = 32;

    template<typename ShaderType>
    struct Shaders
    {
      // TODO: Create proper lookup data structure.
      CComPtr<ShaderType> handles[shaderCountMax];
      LPCTSTR names[shaderCountMax];
      TCHAR namesBuffer[shaderCountMax * shaderNameLengthMax]; // TODO: string hashes for faster lookup?
      int64 namesBufferEnd; // 1 past last character.
      int64 count;

      void reset();
      void addName(LPCTSTR fileName);
    };

    struct VertexShaders : public Shaders<ID3D11VertexShader>
    {
      CComPtr<ID3D11InputLayout> inputLayouts[shaderCountMax];
    };

    VertexShaders vertexShaders;
    Shaders<ID3D11PixelShader> pixelShaders;

    void reset();

    void load(LPCTSTR shaderDirectoryPath, std::vector<byte>& shaderBytecode);

    void loadVertexShaderSourceFile(LPCTSTR filePath, LPCTSTR fileName);
    void loadVertexShaderObjectFile(LPCTSTR filePath, LPCTSTR fileName, std::vector<byte>& shaderBytecode);
    void createVertexShader(LPCTSTR fileName, const void* shaderBytecode, uint64 shaderBytecodeSize);

    void loadPixelShaderSourceFile(LPCTSTR filePath, LPCTSTR fileName);
    void loadPixelShaderObjectFile(LPCTSTR filePath, LPCTSTR fileName, std::vector<byte>& shaderBytecode);
    void createPixelShader(LPCTSTR fileName, const void* shaderBytecode, uint64 shaderBytecodeSize);

  };
}