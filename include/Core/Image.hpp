#pragma once

#include "Core/Core.hpp"

struct PngReadResult
{
  PngReadResult() = default;
  PngReadResult(const PngReadResult& other) = delete;
  PngReadResult(PngReadResult&& other) noexcept;
  ~PngReadResult();

  PngReadResult& operator=(PngReadResult&& other) noexcept;

  friend void swap(PngReadResult& first, PngReadResult& second);

  byte* data = nullptr; // Feel free to steal this pointer. Free it by calling free() on it. Remember to set it to nullptr.
  int64 width = 0;
  int64 height = 0;
  int64 channelCount = 0;
};
PngReadResult readPng(const byte* pngData, int64 pngDataSize);
PngReadResult readPng(const wchar_t* fileName);

// data is expected to contain big endian in case of 16+ bit depths.
bool writePngGrayscaleBigEndian(const char* fileName, const byte* data, int64 width, int64 height, int64 channelCount, int64 bitDepth);