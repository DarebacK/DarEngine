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

  // TODO: Investigate using D3DReflect instead of manually specifying input layout, like this:
  //HRESULT CreateInputLayoutDescFromVertexShaderSignature(ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout)
  //{
  //  // Reflect shader info
  //  ID3D11ShaderReflection* pVertexShaderReflection = NULL;
  //  if(FAILED(D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)))
  //  {
  //    return S_FALSE;
  //  }

  //  // Get shader info
  //  D3D11_SHADER_DESC shaderDesc;
  //  pVertexShaderReflection->GetDesc(&shaderDesc);

  //  // Read input layout description from shader info
  //  std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
  //  for(uint32 i = 0; i < shaderDesc.InputParameters; i++)
  //  {
  //    D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
  //    pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

  //    // fill out input element desc
  //    D3D11_INPUT_ELEMENT_DESC elementDesc;
  //    elementDesc.SemanticName = paramDesc.SemanticName;
  //    elementDesc.SemanticIndex = paramDesc.SemanticIndex;
  //    elementDesc.InputSlot = 0;
  //    elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  //    elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  //    elementDesc.InstanceDataStepRate = 0;

  //    // determine DXGI format
  //    if(paramDesc.Mask == 1)
  //    {
  //      if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
  //      else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
  //      else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
  //    }
  //    else if(paramDesc.Mask <= 3)
  //    {
  //      if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
  //      else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
  //      else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
  //    }
  //    else if(paramDesc.Mask <= 7)
  //    {
  //      if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
  //      else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
  //      else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
  //    }
  //    else if(paramDesc.Mask <= 15)
  //    {
  //      if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
  //      else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
  //      else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  //    }

  //    //save element desc
  //    inputLayoutDesc.push_back(elementDesc);
  //  }

  //  // Try to create Input Layout
  //  HRESULT hr = pD3DDevice->CreateInputLayout(&inputLayoutDesc[0], inputLayoutDesc.size(), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pInputLayout);

  //  //Free allocation shader reflection memory
  //  pVertexShaderReflection->Release();
  //  return hr;
  //}

  #define PIXEL_SHADER(name) extern CComPtr<ID3D11PixelShader> name ## PixelShader ;
  #include PIXEL_SHADERS
  #undef PIXEL_SHADER
  #define PIXEL_SHADER(name) + 1
  inline constexpr int64 pixelShaderCount = 0
  #include PIXEL_SHADERS
  ;
  #undef PIXEL_SHADER
}