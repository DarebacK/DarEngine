#include "Core/Image.hpp"

#include "Core/Core.hpp"
#include "Core/File.hpp"

#include <external/lodepng.h>

PngReadResult::PngReadResult(PngReadResult&& other) noexcept
{
  data = other.data;
  other.data = nullptr;

  width = other.width;
  height = other.height;
  channelCount = other.channelCount;
}

PngReadResult::~PngReadResult()
{
  if (data)
  {
    free(data);
  }
}

PngReadResult& PngReadResult::operator=(PngReadResult&& other) noexcept
{
  swap(*this, other);
  return *this;
}

void swap(PngReadResult& first, PngReadResult& second)
{
  using std::swap;

  swap(first.data, second.data);
  swap(first.width, second.width);
  swap(first.height, second.height);
  swap(first.channelCount, second.channelCount);
}

PngReadResult readPng(const byte* pngData, int64 pngDataSize)
{
  PngReadResult result;

  unsigned int width, height;
  LodePNGState state;
  lodepng_state_init(&state);
  if (lodepng_inspect(&width, &height, &state, pngData, pngDataSize) != 0)
  {
    logError("Failed to inspect PNG header.");
    return {};
  }

  if (lodepng_decode_memory(reinterpret_cast<unsigned char**>(&result.data), &width, &height, pngData, pngDataSize, state.info_png.color.colortype, state.info_png.color.bitdepth) != 0)
  {
    logError("Failed to read PNG data from memory.");
    lodepng_state_cleanup(&state);
    return {};
  }

  result.channelCount = lodepng_get_channels(&state.info_png.color);
  result.height = height;
  result.width = width;

  lodepng_state_cleanup(&state);

  return result;
}

PngReadResult readPng(const wchar_t* fileName)
{
  std::vector<byte> fileData;
  if (!tryReadEntireFile(fileName, fileData))
  {
    logError("Failed to read png file %ls", fileName);
    return {};
  }

  return readPng(fileData.data(), fileData.size());
}

bool writePngGrayscaleBigEndian(const char* fileName, const byte* data, int64 width, int64 height, int64 channelCount, int64 bitDepth)
{
  lodepng::State state;

  LodePNGColorMode colorMode = lodepng_color_mode_make(LodePNGColorType::LCT_GREY, static_cast<unsigned int>(bitDepth));
  lodepng_color_mode_copy(&state.info_raw, &colorMode);
  lodepng_color_mode_copy(&state.info_png.color, &colorMode);

  state.encoder.auto_convert = 0;

  return lodepng::encode(fileName, data, static_cast<unsigned int>(width), static_cast<unsigned int>(height), state) == 0;
}