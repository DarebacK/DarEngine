#define DAR_MODULE_NAME "D3D11"

#include "Core/D3D11.hpp"

#include "external/DirextXTex/DDSTextureLoader11.h"

using namespace D3D11;

HRESULT createTextureFromDDS(const uint8* ddsData, uint64 ddsDataSize, ID3D11Resource** texture, ID3D11ShaderResourceView** textureView)
{
  return DirectX::CreateDDSTextureFromMemoryEx(device, ddsData, ddsDataSize, 0, D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, 0, 0, DirectX::DDS_LOADER_DEFAULT, texture, textureView);
}

PixelFormat toPixelFormat(DXGI_FORMAT format)
{
  switch(format)
  {
    case DXGI_FORMAT_R8G8B8A8_UNORM: return PixelFormat::RGBA;
    case DXGI_FORMAT_BC1_UNORM: return PixelFormat::BC1;

    default:
      ensureNoEntry();
      return PixelFormat::Invalid;
  }
}