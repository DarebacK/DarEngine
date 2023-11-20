#pragma once

#include "Core/D3D11.hpp"

namespace D3D11
{
  void reloadShaders(LPCTSTR shaderDirectoryPath);

  #define VERTEX_SHADER(name, ...) extern CComPtr<ID3D11VertexShader> name ## VertexShader ;
  #include VERTEX_SHADERS
  #undef VERTEX_SHADER
  #define VERTEX_SHADER(name, ...) + 1
  inline constexpr int64 vertexShaderCount = 0
  #include VERTEX_SHADERS
  ;
  #undef VERTEX_SHADER
  #define VERTEX_SHADER(name, ...) extern CComPtr<ID3D11InputLayout> name ## InputLayout ;
  #include VERTEX_SHADERS
  #undef VERTEX_SHADER

  // TODO: Investigate using D3DReflect instead of manually specifying input layout

  #define PIXEL_SHADER(name) extern CComPtr<ID3D11PixelShader> name ## PixelShader ;
  #include PIXEL_SHADERS
  #undef PIXEL_SHADER
  #define PIXEL_SHADER(name) + 1
  inline constexpr int64 pixelShaderCount = 0
  #include PIXEL_SHADERS
  ;
  #undef PIXEL_SHADER
}