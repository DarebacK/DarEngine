#pragma once

#include "Core/Math.hpp"

struct Mouse
{
  struct Button
  {
    bool pressedDown;
    bool isDown;
    bool pressedUp;
  };
  Button left;
  Button middle;
  Button right;
  float dWheel;
};

struct Keyboard
{
  struct Key
  {
    bool pressedDown; // Whether the key was just pressed down.
    bool isDown; // Whether the key is being held down. 
    bool pressedUp; // Whether the key was just pressed up.
  };

  #define KEYBOARD_KEY(vk, fieldName) Key fieldName;
    #include "Core/Input.inl"
  #undef KEYBOARD_KEY
};

struct Input
{
  Mouse mouse;
  Vec2i cursorPosition;
  Keyboard keyboard;

  // Returns true if it was an input message and was processed.
  bool processMessage(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
  void resetForNextFrame();
};