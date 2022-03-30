#pragma once

#include "Core/Core.hpp"
#include "Core/Math.hpp"

class MainWindow
{
public:

  MainWindow() = default;
  MainWindow(const MainWindow& other) = delete;
  MainWindow(MainWindow&& other) = delete;
  ~MainWindow() = default;

  bool tryInitialize(HINSTANCE instance, const wchar_t* name, WNDPROC wndProc);

  operator HWND() { return handle; }

  void show();
  void showErrorMessageBox(const wchar_t* text, const wchar_t* caption);

  Vec2i getCursorPosition();

private:

  HWND handle = nullptr;

};
