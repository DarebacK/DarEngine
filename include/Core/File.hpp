#pragma once

#include "Core/Core.hpp"

#include <vector>

bool tryReadEntireFile(const wchar_t* fileName, std::vector<byte>& buffer);