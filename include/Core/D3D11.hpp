#pragma once

#include <d3d11_4.h>
#include <atlbase.h>

#include "Core/Core.hpp"

namespace D3D11
{
  extern CComPtr<ID3D11Device> device;
  extern CComPtr<ID3D11DeviceContext> context;
}

HRESULT createTextureFromDDS(const uint8* ddsData, uint64 ddsDataSize, ID3D11Resource** texture, ID3D11ShaderResourceView** textureView);