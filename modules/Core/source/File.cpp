#include "Core/File.hpp"

#include <fstream>

bool tryReadEntireFile(const wchar_t* fileName, std::vector<byte>& buffer)
{
  std::ifstream file(fileName, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    return false;
  }

  const size_t fileSize = file.tellg();
  buffer.resize(fileSize);

  file.seekg(0);
  file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

  return true;
}