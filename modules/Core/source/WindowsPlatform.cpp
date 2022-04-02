#include "Core/WindowsPlatform.h"

bool MainWindow::tryInitializeGameStyle(HINSTANCE instance, const wchar_t* name, WNDPROC wndProc)
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

bool MainWindow::tryInitializeEditorStyle(HINSTANCE instance, const wchar_t* name, WNDPROC wndProc)
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
  ShowWindow(handle, SW_SHOWMAXIMIZED);
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

void runGameLoop(std::function<void(int64 frameIndex, float timeDelta)> frameCallback)
{
  LARGE_INTEGER counterFrequency;
  QueryPerformanceFrequency(&counterFrequency);
  LARGE_INTEGER lastCounterValue;
  QueryPerformanceCounter(&lastCounterValue);

  int64 frameIndex = 0;

  MSG message{};
  while (message.message != WM_QUIT) {
    if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&message);
      DispatchMessage(&message);
    }
    else {
      LARGE_INTEGER currentCounterValue;
      QueryPerformanceCounter(&currentCounterValue);
      const float timeDelta = (float)(currentCounterValue.QuadPart - lastCounterValue.QuadPart) / counterFrequency.QuadPart;
      lastCounterValue = currentCounterValue;

      frameCallback(frameIndex++, timeDelta);
    }
  }
}