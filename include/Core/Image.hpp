#pragma once

#include "Core/Core.hpp"

struct PngReadResult
{
  PngReadResult() = default;
  PngReadResult(const PngReadResult& other) = delete;
  PngReadResult(PngReadResult&& other);
  ~PngReadResult();

  byte* data;
  int64 width;
  int64 height;
  int64 channelCount;
};
PngReadResult readPng(const byte* pngData, int64 pngDataSize);

// You can leave strideInBytes to 0 for 1 byte per channel.
bool writePng(const char* fileName, const byte* data, int64 width, int64 height, int64 channelCount, int64 strideInBytes = 0);