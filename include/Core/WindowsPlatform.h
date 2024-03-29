#pragma once

#include "Core/Core.hpp"
#include "Core/Math.hpp"

#include <functional>

class MainWindow
{
public:

  MainWindow() = default;
  MainWindow(const MainWindow& other) = delete;
  MainWindow(MainWindow&& other) = delete;
  ~MainWindow() = default;

  bool tryInitializeGameStyle(HINSTANCE instance, const wchar_t* name, WNDPROC wndProc);
  bool tryInitializeEditorStyle(HINSTANCE instance, const wchar_t* name, WNDPROC wndProc);

  operator HWND() { return handle; }

  void show();
  void showErrorMessageBox(const wchar_t* text, const wchar_t* caption);

  Vec2i getCursorPosition();

private:

  HWND handle = nullptr;

};

void runGameLoop(std::function<void(int64 frameIndex, float timeDelta)> frameCallback);

bool tryChooseFolderDialog(HWND window, const wchar_t* title, wchar_t* path);
