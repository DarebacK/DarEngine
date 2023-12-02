#include "Core/WindowsPlatform.h"

#include <Shlobj_core.h>

bool MainWindow::tryInitializeGameStyle(HINSTANCE instance, const wchar_t* name, WNDPROC wndProc)
{
  TRACE_SCOPE();

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

bool MainWindow::tryInitializeEditorStyle(HINSTANCE instance, const wchar_t* name, WNDPROC wndProc)
{
  TRACE_SCOPE();

  WNDCLASS windowClass{};
  windowClass.lpfnWndProc = wndProc;
  windowClass.hInstance = instance;
  windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
  windowClass.lpszClassName = L"MainWindow class";
  if (!RegisterClass(&windowClass)) {
    showErrorMessageBox(L"Failed to register window class.", L"Fatal error");
    return false;
  }

  RECT workArea;
  if (!SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0))
  {
    logError("Failed to get system work area. Expect wrong main window size.");
    workArea.left = 0;
    workArea.right = GetSystemMetrics(SM_CXSCREEN);
    workArea.top = 0;
    workArea.bottom = GetSystemMetrics(SM_CYSCREEN);
  }

  constexpr DWORD windowStyle = WS_OVERLAPPEDWINDOW | WS_MAXIMIZE;
  constexpr DWORD windowStyleEx = 0;
  handle = CreateWindowEx(
    windowStyleEx,
    windowClass.lpszClassName,
    name,
    windowStyle,
    0,
    0,
    workArea.right,
    workArea.bottom,
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
  TRACE_SCOPE();
  ShowWindow(handle, SW_SHOWMAXIMIZED);
}

void MainWindow::showErrorMessageBox(const wchar_t* text, const wchar_t* caption)
{
  TRACE_SCOPE();
  MessageBox(handle , text, caption, MB_OK | MB_ICONERROR);
}

Vec2i MainWindow::getCursorPosition()
{
  TRACE_SCOPE();

  POINT mousePosition;
  if (!GetCursorPos(&mousePosition)) {
    return {};
  }
  if (!ScreenToClient(handle, &mousePosition)) {
    return {};
  }
  return Vec2i{ mousePosition.x, mousePosition.y };
}

void runGameLoop(std::function<void(int64 frameIndex, float timeDelta)> frameCallback)
{
  LARGE_INTEGER counterFrequency;
  QueryPerformanceFrequency(&counterFrequency);
  LARGE_INTEGER lastCounterValue;
  QueryPerformanceCounter(&lastCounterValue);

  int64 frameIndex = 0;

  MSG message{};
  while (true)
  {
    TRACE_FRAME();

    {
      TRACE_SCOPE("pumpWindowsMessages");

      while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);

        if (message.message == WM_QUIT)
        {
          return;
        }
      }
    }

    LARGE_INTEGER currentCounterValue;
    QueryPerformanceCounter(&currentCounterValue);
    const float timeDelta = (float)(currentCounterValue.QuadPart - lastCounterValue.QuadPart) / counterFrequency.QuadPart;
    lastCounterValue = currentCounterValue;

    frameCallback(frameIndex++, timeDelta);
  }
}

bool tryChooseFolderDialog(HWND window, const wchar_t* title, wchar_t* path)
{
  TRACE_SCOPE();

  BROWSEINFOW info = {};
  info.hwndOwner = window;
  info.pidlRoot = NULL;
  wchar_t displayName[MAX_PATH];
  info.pszDisplayName = displayName;
  info.lpszTitle = title;
  info.ulFlags = BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
  LPITEMIDLIST pidl = SHBrowseForFolder(&info);
  if (!pidl)
  {
    return false;
  }

  if (!SHGetPathFromIDList(pidl, path))
  {
    logError("Failed to get folder path for %ls.", displayName);
    CoTaskMemFree(pidl);
    return false;
  }

  CoTaskMemFree(pidl);
  return true;
}