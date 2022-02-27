#pragma once

#include "Core/Core.hpp"
#include "D3D11/D3D11.hpp"

class TaskScheduler;

namespace D3D11
{
  class ShaderRegistry
  {
  public:

    ShaderRegistry();

    void reload(LPCTSTR shaderDirectoryPath, TaskScheduler* taskScheduler);

    ID3D11VertexShader* findVertexShader(LPCTSTR name) const;
    ID3D11PixelShader* findPixelShader(LPCTSTR name) const;

  private:
    
    void reset();
    void load(LPCTSTR shaderDirectoryPath, TaskScheduler* taskScheduler);

    static constexpr int shaderCountMax = 50;

    // TODO: Create proper lookup data structure.
    LPCTSTR vertexShaderNames[shaderCountMax];
    ID3D11VertexShader* vertexShaders[shaderCountMax];
    int64 vertexShaderCount;

    LPCTSTR pixelShaderNames[shaderCountMax];
    ID3D11PixelShader* pixelShaders[shaderCountMax];
    int64 pixelShaderCount;
  };
}