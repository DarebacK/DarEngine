#include "Core/Input.hpp"

#define VK_Q 0x51
#define VK_W 0x57
#define VK_E 0x45
#define VK_A 0x41
#define VK_S 0x53
#define VK_D 0x44

#define OnKeyDown(vk, key) case vk: \
  { \
    if(!keyboard.key.isDown) \
    { \
      keyboard.key.pressedDown = true; \
      keyboard.key.isDown = true; \
    } \
    return true; \
  }

#define OnKeyUp(vk, key) case vk: \
  { \
    keyboard.key.isDown = false; \
    keyboard.key.pressedUp = true; \
    return true; \
  }

bool Input::processMessage(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case WM_LBUTTONDOWN:
      mouse.left.pressedDown = true;
      mouse.left.isDown = true;
      return true;
      break;
    case WM_LBUTTONUP:
      mouse.left.pressedUp = true;
      mouse.left.isDown = false;
      return true;
      break;
    case WM_MBUTTONDOWN:
      mouse.middle.pressedDown = true;
      mouse.middle.isDown = true;
      return true;
      break;
    case WM_MBUTTONUP:
      mouse.middle.pressedUp = true;
      mouse.middle.isDown = false;
      return true;
      break;
    case WM_RBUTTONDOWN:
      mouse.right.pressedDown = true;
      mouse.right.isDown = true;
      return true;
      break;
    case WM_RBUTTONUP:
      mouse.right.pressedUp = true;
      mouse.right.isDown = false;
      return true;
      break;
    case WM_MOUSEWHEEL:
      mouse.dWheel += (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
      return true;
      break;
    case WM_KEYDOWN:
      switch (wParam) {
        case VK_ESCAPE:
          PostQuitMessage(0);
          break;

          #define KEYBOARD_KEY(vk, fieldName) OnKeyDown(vk, fieldName)
          #include "Core/Input.inl"
          #undef KEYBOARD_KEY
        }
        break;
    case WM_KEYUP:
      switch (wParam) {
        #define KEYBOARD_KEY(vk, fieldName) OnKeyUp(vk, fieldName)
        #include "Core/Input.inl"
        #undef KEYBOARD_KEY
      }
      break;
  }

  return false;
}

void Input::resetForNextFrame()
{
  mouse.left.pressedDown = mouse.left.pressedUp = false;
  mouse.middle.pressedDown = mouse.left.pressedUp = false;
  mouse.right.pressedDown = mouse.right.pressedUp = false;
  mouse.dWheel = 0.f;

  #define KEYBOARD_KEY(vk, fieldName) \
    keyboard.fieldName.pressedDown = keyboard.fieldName.pressedUp = false;
  #include "Core/Input.inl"
  #undef KEYBOARD_KEY
}