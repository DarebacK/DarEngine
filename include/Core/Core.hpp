#pragma once
#include <stdio.h>
#include <cstdint>

using byte = uint8_t; static_assert(sizeof(byte) == 1);
using int8 = int8_t; static_assert(sizeof(int8) == 1);
using int16 = int16_t; static_assert(sizeof(int16) == 2);
using int32 = int32_t; static_assert(sizeof(int32) == 4);
using int64 = int64_t; static_assert(sizeof(int64) == 8);
using uint8 = uint8_t; static_assert(sizeof(byte) == 1);
using uint16 = uint16_t; static_assert(sizeof(uint16) == 2);
using uint32 = uint32_t; static_assert(sizeof(uint32) == 4);
using uint64 = uint64_t; static_assert(sizeof(uint64) == 8);

#ifndef DAR_MODULE_NAME
  #define DAR_MODULE_NAME ""
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

  #define logError(message, ...) \
  { \
    char stringBuffer[1024]; \
    _snprintf_s(stringBuffer, sizeof(stringBuffer), "[ERROR][" DAR_MODULE_NAME "] " message "\n", __VA_ARGS__); \
    OutputDebugStringA(stringBuffer); \
  }
  #define logWarning(message, ...) \
  { \
    char stringBuffer[1024]; \
    _snprintf_s(stringBuffer, sizeof(stringBuffer), "[WARN][" DAR_MODULE_NAME "] " message "\n", __VA_ARGS__); \
    OutputDebugStringA(stringBuffer); \
  }
  #define logInfo(message, ...) \
  { \
    char stringBuffer[1024]; \
    _snprintf_s(stringBuffer, sizeof(stringBuffer), "[INFO][" DAR_MODULE_NAME "] " message "\n", __VA_ARGS__); \
    OutputDebugStringA(stringBuffer); \
  }
#endif
#define logVariable(variable, format) logInfo(#variable " = " format, variable)

#define arrayLength(arr) (sizeof(arr) / sizeof(arr[0]))

#ifdef DAR_DEBUG
  #undef assert
  #define assert(condition) \
    if(!(condition)) { \
      logError("Assertion failed: %s", #condition); \
      *(int*)0 = 0; \
    }

  extern wchar_t _debugText[4096];
  extern int _debugTextLength;

  #define debugResetText() _debugTextLength = 0;

  void _debugStringImpl(const wchar_t* newStr, int newStrLength);
  #define debugText(...) \
  { \
    wchar_t newStr[256]; \
    int newStrLength = _snwprintf_s(newStr, _TRUNCATE, __VA_ARGS__); \
    if(newStrLength > 0) _debugStringImpl(newStr, newStrLength); \
  }
#else 
  #undef assert
  #define assert(condition) ((void)0)
  #define debugText(...)
  #define debugResetText()
#endif