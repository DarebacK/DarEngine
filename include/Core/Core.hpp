#pragma once

#include <stdlib.h>
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
#define NOMINMAX
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

#define PLATFORM_WINDOWS 1 // Only supported platform so far

#if PLATFORM_WINDOWS
  #define LITTLE_ENDIAN 1
  #define BIG_ENDIAN 0
#endif

#ifdef _MSC_VER
  #define COMPILER_MSVC 1
#else
  #define COMPILER_MSVC 0
#endif

inline uint16 nativeToBigEndian(uint16 value)
{
#if BIG_ENDIAN
    return value;
#elif COMPILER_MSVC
    return _byteswap_ushort(value);
#endif
}
inline uint16 bigEndianToNative(uint16 value)
{
#if BIG_ENDIAN
  return value;
#elif COMPILER_MSVC
  return _byteswap_ushort(value);
#endif
}

inline int16 nativeToBigEndian(int16 value)
{
#if BIG_ENDIAN
  return value;
#elif COMPILER_MSVC
  return _byteswap_ushort(value);
#endif
}
inline int16 bigEndianToNative(int16 value)
{
#if BIG_ENDIAN
  return value;
#elif COMPILER_MSVC
  return _byteswap_ushort(value);
#endif
}

#define STRINGIFY(a) #a
#define STRINGIFY_DEFINE(a) STRINGIFY(a)

#define WSTRINGIFY(a) L ## #a
#define WSTRINGIFY_DEFINE(a) WSTRINGIFY(a)

#include "external/optick/optick.h"
#define TRACE_FRAME() OPTICK_FRAME("MainThread")
#define TRACE_SCOPE(...) OPTICK_EVENT(__VA_ARGS__)
#define TRACE_THREAD(name) OPTICK_THREAD(name)
#define TRACE_START_CAPTURE() OPTICK_START_CAPTURE();
#define TRACE_STOP_CAPTURE(...) OPTICK_STOP_CAPTURE(); OPTICK_SAVE_CAPTURE(__VA_ARGS__);

#define CACHE_LINE_SIZE 64
