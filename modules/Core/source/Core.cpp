#include "Core/Core.hpp"

#include "Core/Math.hpp"

#ifdef DAR_DEBUG
wchar_t _debugText[4096];
int _debugTextLength = 0;

void _debugStringImpl(const wchar_t* newStr, int newStrLength)
{
  int newDebugTextStringLength = _debugTextLength + newStrLength;
  newDebugTextStringLength = std::clamp(newDebugTextStringLength, 0, (int)arrayLength(_debugText));
  wchar_t* debugTextStringOffset = _debugText + _debugTextLength;
  int remainingDebugTextStringSpace =  arrayLength(_debugText) - _debugTextLength;
  if(remainingDebugTextStringSpace > 0)
  {
    _snwprintf_s(debugTextStringOffset, arrayLength(_debugText) - _debugTextLength, _TRUNCATE, L"%s\n", newStr);
    _debugTextLength = newDebugTextStringLength + 1;
  }
}
#endif