#include "DarEngine.hpp"

#include "DarMath.hpp"

#ifdef DAR_DEBUG
wchar_t _debugText[4096];
int _debugTextLength = 0;

void _debugStringImpl(const wchar_t* newStr, int newStrLength)
{
  int newDebugTextStringLength = _debugTextLength + newStrLength;
  newDebugTextStringLength = std::clamp(newDebugTextStringLength, 0, (int)arrayCount(_debugText));
  wchar_t* debugTextStringOffset = _debugText + _debugTextLength;
  int remainingDebugTextStringSpace =  arrayCount(_debugText) - _debugTextLength;
  if(remainingDebugTextStringSpace > 0)
  {
    _snwprintf_s(debugTextStringOffset, arrayCount(_debugText) - _debugTextLength, _TRUNCATE, L"%s\n", newStr);
    _debugTextLength = newDebugTextStringLength + 1;
  }
}
#endif