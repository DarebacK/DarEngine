#include "Core/WindowsPlatform.h"

bool MainWindow::tryInitialize(HINSTANCE instance, const wchar_t* name, WNDPROC wndProc)
{
  WNDCLASS windowClass{};
  windowClass.lpfnWndProc = wndProc;
  windowClass.hInstance = instance;
  windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  windowClass.lpszClassName = L"MainWindow class";
  if (!RegisterClass(&windowClass)) {
    showErrorMessageBox(L"Failed to register window class.", L"Fatal error");
    return false;
  }

  const int clientAreaWidth = GetSystemMetrics(SM_CXSCREEN);
  const int clientAreaHeight = GetSystemMetrics(SM_CYSCREEN);

  RECT windowRectangle = { 0, 0, clientAreaWidth, clientAreaHeight };
  constexpr DWORD windowStyle = WS_POPUP;
  constexpr DWORD windowStyleEx = 0;
  AdjustWindowRectEx(&windowRectangle, windowStyle, false, windowStyleEx);
  const int windowWidth = windowRectangle.right - windowRectangle.left;
  const int windowHeight = windowRectangle.bottom - windowRectangle.top;
  handle = CreateWindowEx(
    windowStyleEx,
    windowClass.lpszClassName,
    name,
    windowStyle,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    windowWidth,
    windowHeight,
    nullptr,
    nullptr,
    windowClass.hInstance,
    nullptr
  );
  if (!handle) {
    showErrorMessageBox(L"Failed to create game window", L"Fatal error");
    return false;
  }

  return true;
}

void MainWindow::show()
{
  ShowWindow(handle, SW_SHOWNORMAL);
}

void MainWindow::showErrorMessageBox(const wchar_t* text, const wchar_t* caption)
{
  MessageBox(handle , text, caption, MB_OK | MB_ICONERROR);
}

Vec2i MainWindow::getCursorPosition()
{
  Vec2i mousePosition;
  if (!GetCursorPos((LPPOINT)&mousePosition)) {
    return {};
  }
  if (!ScreenToClient(handle , (LPPOINT)&mousePosition)) {
    return {};
  }
  return mousePosition;
}