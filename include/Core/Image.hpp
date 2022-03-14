#pragma once

#include "Core/Core.hpp"

struct PngReadResult
{
  PngReadResult() = default;
  PngReadResult(const PngReadResult& other) = delete;
  PngReadResult(PngReadResult&& other);
  ~PngReadResult();

  PngReadResult& operator=(PngReadResult&& other);

  friend void swap(PngReadResult& first, PngReadResult& second);

  byte* data = nullptr;
  int64 width = 0;
  int64 height = 0;
  int64 channelCount = 0;
};
PngReadResult readPng(const byte* pngData, int64 pngDataSize);
PngReadResult readPng(const wchar_t* fileName);

// data is expected to contain big endian in case of 16+ bit depths.
bool writePngLosslessGrayscaleBigEndian(const char* fileName, const byte* data, int64 width, int64 height, int64 channelCount, int64 bitDepth);