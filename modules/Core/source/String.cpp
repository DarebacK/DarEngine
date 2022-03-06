#include "Core/String.hpp"

int64 combine(LPCTSTR string1, LPCTSTR string2, LPTSTR destination)
{
  int64 length = 0;
  while (string1[length] != L'\0')
  {
    destination[length] = string1[length];
    length++;
  }
  destination[length++] = L'\\';
  int64 string2Index = 0;
  while (string2[string2Index] != L'\0')
  {
    destination[length++] = string2[string2Index++];
  }
  return length;
}