#include "Core/Image.hpp"

#include "Core/Core.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <external/stb_image.h>

#include <external/lodepng.h>

PngReadResult::PngReadResult(PngReadResult&& other)
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
    stbi_image_free(data);
  }
}

PngReadResult readPng(const byte* pngData, int64 pngDataSize)
{
  PngReadResult result;
  
  int width, height, channelCount;
  result.data = stbi_load_from_memory(pngData, static_cast<int>(pngDataSize), &width, &height, &channelCount, 0);
  if (!result.data)
  {
    logError("Failed to read PNG data from memory.");
    return {};
  }

  result.width = width;
  result.height = height;
  result.channelCount = channelCount;
  return result;
}

bool writePngLosslessGrayscaleBigEndian(const char* fileName, const byte* data, int64 width, int64 height, int64 channelCount, int64 bitDepth)
{
  lodepng::State state;

  LodePNGColorMode colorMode = lodepng_color_mode_make(LodePNGColorType::LCT_GREY, bitDepth);
  lodepng_color_mode_copy(&state.info_raw, &colorMode);
  lodepng_color_mode_copy(&state.info_png.color, &colorMode);

  state.encoder.auto_convert = 0;

  return lodepng::encode(fileName, data, width, height, state) == 0;
}