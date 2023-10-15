#include "Core/String.hpp"

int64 combine(const wchar_t* string1, wchar_t char2, const wchar_t* string3, wchar_t* destination)
{
  int64 length = 0;
  while (string1[length] != L'\0')
  {
    destination[length] = string1[length];
    length++;
  }
  destination[length++] = char2;
  int64 string3Index = 0;
  while (string3[string3Index] != L'\0')
  {
    destination[length++] = string3[string3Index++];
  }
  return length;
}