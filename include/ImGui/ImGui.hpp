#pragma once

#define IMGUI_USER_CONFIG <external/imconfig.h>
#include <external/imgui.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Dar
{
  class ImGui
  {
  public:

    ImGui(void* windowHandle, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
    ImGui(const ImGui& other) = delete;
    ImGui(ImGui&& other) = delete;
    ~ImGui();

    void newFrame();
    void render();
  };
}