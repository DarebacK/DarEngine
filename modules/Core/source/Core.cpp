#include "Core/Core.hpp"

#include <unordered_map>

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

// Enum ********************************************************************************************

// ! Do not access in static object's constructor !
const std::unordered_map<const char*, void*>* enumNameToToEnumFunctionPtr;

EnumRegisterer::EnumRegisterer(const char* name, void* toEnum)
{
  // Ensures that enumNameToToEnumFunction is initialized before we access it.
  static std::unordered_map<const char*, void*> enumNameToToEnumFunction;
  enumNameToToEnumFunctionPtr = &enumNameToToEnumFunction;

  auto itInsertedPair = enumNameToToEnumFunction.try_emplace(name, toEnum);
  ensureTrue(itInsertedPair.second);
}
void* findToEnumFunction(const char* enumName)
{
  auto it = enumNameToToEnumFunctionPtr->find(enumName);
  if(it == enumNameToToEnumFunctionPtr->end())
  {
    ensureNoEntry();
    return nullptr;
  }

  return it->second;
}