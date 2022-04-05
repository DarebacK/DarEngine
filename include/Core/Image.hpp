#pragma once

#include "Core/Core.hpp"

// General -----------------------------------------------------------------------------------------

enum class PixelFormat : uint8
{
  Invalid = 0,

  RGBA,
  BGRA,
  ARGB,
  ABGR,
  CMYK,
  RGB,
  BGR,
  Grayscale,
};
int8 toChannelCount(PixelFormat pixelFormat);

struct RgbPixel
{
  uint8 r, g, b;
};

enum ChromaSubsampling : uint8
{
  ChromaSubsampling_Invalid = 0,

  ChromaSubsampling_444,
  ChromaSubsampling_440,
  ChromaSubsampling_422,
  ChromaSubsampling_420,
  ChromaSubsampling_411,
  ChromaSubsampling_Grayscale,
};

struct Image
{
  Image() = default;
  Image(const Image& other) = delete;
  Image(Image&& other) noexcept;
  ~Image();

  Image& operator=(Image&& other) noexcept;

  friend void swap(Image& first, Image& second);

  int64 getDataSize() const;

  byte* data = nullptr; // Feel free to steal this pointer. Free it by calling free() on it. Remember to set it to nullptr.
  int32 width = 0;
  int32 height = 0;
  PixelFormat pixelFormat = PixelFormat::Invalid;
  ChromaSubsampling chromaSubsampling = ChromaSubsampling_Invalid;
};

// PNG ---------------------------------------------------------------------------------------------

// TODO: Use Image struct instead
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

// JPEG --------------------------------------------------------------------------------------------

class JpegReader
{
public:

  JpegReader();
  JpegReader(const JpegReader& other) = delete;
  JpegReader(JpegReader&& other);
  ~JpegReader();

  // returned Image.chromaSubsampling contains the input subsampling. 
  Image read(const wchar_t* filePath, PixelFormat outputPixelFormat);
  Image read(const byte* jpegData, int64 jpegDataSize, PixelFormat outputPixelFormat);

private:

  void* decompressor = nullptr;
};

class JpegWriter
{
public:

  JpegWriter();
  JpegWriter(const JpegWriter& other) = delete;
  JpegWriter(JpegWriter&& other) = delete;
  ~JpegWriter();

  /**
   * @param input input.chromaSubsampling has to contain required chroma subsampling.
   * @param quality ranges 0 - 100.
   */ 
  bool tryWrite(const Image& input, int8 quality, const char* fileName);

private:

  void* compressor;
};

// WEBP --------------------------------------------------------------------------------------------

class WebpReader
{
public:

  WebpReader() = default;
  WebpReader(const WebpReader& other) = delete;
  WebpReader(WebpReader&& other) = delete;
  ~WebpReader() = default;

  Image read(const byte* data, int64 dataSize, PixelFormat outputPixelFormat);

};