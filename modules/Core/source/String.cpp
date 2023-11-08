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

bool contains(const wchar_t* string1, const wchar_t* string2, int64 lengthToCompare)
{
  ensureTrue(string1 != nullptr, false);
  ensureTrue(string2 != nullptr, false);

  for (int64 i = 0; i < lengthToCompare; ++i)
  {
    if (string1[i] != string2[i])
    {
      return false;
    }
  }

  return true;
}

const wchar_t* findSubstring(const wchar_t* string, int64 stringLength, const wchar_t* substring, int64 substringLength)
{
  if(!ensure(substringLength <= stringLength))
  {
    return nullptr;
  }

  while(stringLength >= substringLength)
  {
    bool matches = true;
    for(int64 substringIndex = 0; substringIndex < substringLength; substringIndex++)
    {
      if(string[substringIndex] != substring[substringIndex])
      {
        matches = false;
        break;
      }
    }

    if(matches)
    {
      return string;
    }

    string++;
    stringLength--;
  }

  return nullptr;
}
const wchar_t* findSubstring(const wchar_t* string, int64 stringLength, const wchar_t* substring)
{
  return findSubstring(string, stringLength, substring, wcslen(substring));
}
const wchar_t* findSubstring(const wchar_t* string, const wchar_t* substring, int64 substringLength)
{
  return findSubstring(string, wcslen(string), substring, substringLength);
}
const wchar_t* findSubstring(const wchar_t* string, const wchar_t* substring)
{
  return findSubstring(string, wcslen(string), substring, wcslen(substring));
}

int64 getLengthWithoutTrailingSlashes(const wchar_t* string)
{
  ensureTrue(string != nullptr, 0);

  uint64 length = wcslen(string);
  while (string[length - 1] == L'\\' || string[length - 1] == L'/')
  {
    length--;
  }

  return length;
}
int64 getLengthUntilFirstSlash(const wchar_t* string)
{
  ensureTrue(string != nullptr, 0);

  int64 length = 0;
  while (string[length] != L'\\' && string[length] != L'/' && string[length] != L'\0')
  {
    length++;
  }

  return length;
}
int64 getLengthUntilLastSlash(const wchar_t* string)
{
  ensureTrue(string != nullptr, 0);

  uint64 length = wcslen(string);
  while (string[length - 1] != L'\\' && string[length - 1] != L'/')
  {
    length--;
  }
  length--;

  return length;
}