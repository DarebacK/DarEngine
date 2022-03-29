#include "ImGui/ImGui.hpp"

#include "Core/Core.hpp"

#include <external/imgui_impl_win32.h>
#include <external/imgui_impl_dx11.h>

namespace Dar
{
  ImGui::ImGui(void* windowHandle, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
  {
    ::ImGui::CreateContext();
    if (!ImGui_ImplWin32_Init(windowHandle))
    {
      logError("Failed to initializer ImGui Win32 implementation.");
    }
    if (!ImGui_ImplDX11_Init(device, deviceContext))
    {
      logError("Failed to initialize ImGui DX11 implementation.");
    }
  }

  ImGui::~ImGui()
  {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ::ImGui::DestroyContext();
  }

  void ImGui::newFrame()
  {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ::ImGui::NewFrame();
  }

  void ImGui::render()
  {
    ::ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(::ImGui::GetDrawData());
  }
}