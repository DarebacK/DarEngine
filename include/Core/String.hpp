#pragma once

#include "Core/Core.hpp"

// Combines strings into the destination buffer, doesn't null terminate.
int64 combine(const wchar_t* string1, wchar_t char2, const wchar_t* string3, wchar_t* destination);

inline bool isEqual(const wchar_t* string1, const wchar_t* string2) { return wcscmp(string1, string2) == 0; }
