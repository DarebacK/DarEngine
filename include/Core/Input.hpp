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
    bool pressedDown;
    bool pressedUp;
  };

  Key enter;
  Key left;
  Key right;
  Key down;
  Key up;
  Key F1;
  Key F5;
  Key rightAlt;
  Key space;
  Key q, w, e;
  Key a, s, d;
};

struct Input
{
  Mouse mouse;
  Vec2i cursorPosition;
  Keyboard keyboard;
};