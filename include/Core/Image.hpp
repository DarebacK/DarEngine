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

// data is expected to contain big endian in case of 16+ bit depths.
bool writePngBigEndian(const char* fileName, const byte* data, int64 width, int64 height, int64 channelCount, int64 bitDepth);