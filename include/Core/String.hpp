#pragma once

#include "Core/Core.hpp"

// Combines strings into the destination buffer, doesn't null terminate.
int64 combine(const wchar_t* string1, wchar_t char2, const wchar_t* string3, wchar_t* destination);

inline bool isEqual(const wchar_t* string1, const wchar_t* string2) { return wcscmp(string1, string2) == 0; }
inline bool isEqual(const char* string1, const char* string2) { return strcmp(string1, string2) == 0; }
bool contains(const wchar_t* string1, const wchar_t* string2, int64 lengthToCompare);

int64 getLengthWithoutTrailingSlashes(const wchar_t* string);
int64 getLengthUntilFirstSlash(const wchar_t* string);