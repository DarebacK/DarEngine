#include "Core/Image.hpp"

#include "Core/Core.hpp"
#include "Core/Math.hpp"
#include "Core/File.hpp"
#include "Core/String.hpp"

#include <external/lodepng.h>
#include <external/turbojpeg.h>
#include <external/libwebp/decode.h>
#include "external/compressonator/compressonator.h"
#include "external/compressonator/DDS_Helpers.h"

// General -----------------------------------------------------------------------------------------

DAR_ENUM_IMPLEMENT(PixelFormat);

int8 toChannelCount(PixelFormat pixelFormat)
{
  switch (pixelFormat)
  {
  case PixelFormat::RGBA: 
  case PixelFormat::BGRA:
  case PixelFormat::ARGB:
  case PixelFormat::ABGR:
  case PixelFormat::CMYK:
    return 4;

  case PixelFormat::RGB:
  case PixelFormat::BGR:
  case PixelFormat::BC1:
    return 3;

  case PixelFormat::Grayscale:
    return 1;

  default: return 0;
  }
}

int16 toPixelSizeInBits(PixelFormat pixelFormat)
{
  switch (pixelFormat)
  {
    case PixelFormat::RGBA:
    case PixelFormat::BGRA:
    case PixelFormat::ARGB:
    case PixelFormat::ABGR:
    case PixelFormat::CMYK:
      return 32;

    case PixelFormat::RGB:
    case PixelFormat::BGR:
      return 24;

    case PixelFormat::BC1:
      return 4;

    case PixelFormat::Grayscale:
      return 8;

    case PixelFormat::int16: return 16;

    default: return 0;
  }
}
int16 toPixelSizeInBytes(PixelFormat pixelFormat)
{
  return toPixelSizeInBits(pixelFormat) / 8;
}

Image::Image(Image&& other) noexcept
{
  swap(*this, other);
}

Image::~Image()
{
  if (data)
  {
    free(data);
  }
}

Image& Image::operator=(Image&& other) noexcept
{
  swap(*this, other);
  return *this;
}

void swap(Image& first, Image& second)
{
  using std::swap;

  swap(first.data, second.data);
  swap(first.width, second.width);
  swap(first.height, second.height);
  swap(first.pixelFormat, second.pixelFormat);
  swap(first.chromaSubsampling, second.chromaSubsampling);
}

int64 Image::getDataSize() const
{
  if (!data)
  {
    return 0;
  }

  return (width * height * toPixelSizeInBits(pixelFormat)) / 8;
}

int64 calculatePitch(PixelFormat pixelFormat, int64 width)
{
  return (width * toPixelSizeInBits(pixelFormat)) / 8;
}

static CMP_FORMAT toCmpFormat(PixelFormat format)
{
  switch (format)
  {
    case PixelFormat::RGBA: return CMP_FORMAT_RGBA_8888;
    case PixelFormat::BGRA: return CMP_FORMAT_BGRA_8888;
    case PixelFormat::ARGB: return CMP_FORMAT_ARGB_8888;
    case PixelFormat::ABGR: return CMP_FORMAT_ABGR_8888;
    case PixelFormat::RGB: return CMP_FORMAT_RGB_888;
    case PixelFormat::BGR: return CMP_FORMAT_BGR_888;
    case PixelFormat::Grayscale: return CMP_FORMAT_R_8;
    case PixelFormat::BC1: return CMP_FORMAT_BC1;
    default: return CMP_FORMAT_Unknown;
  }
}

Image ImageEncoder::ToBC1Parallel(const Image& source, int64 quality)
{
  ensureTrue(source.getDataSize() >= 0, {});
  ensureTrue(source.width >= 0, {});

  CMP_Texture sourceTexture{};
  sourceTexture.dwSize = sizeof(CMP_Texture);
  sourceTexture.dwWidth = source.width;
  sourceTexture.dwHeight = source.height;
  sourceTexture.dwPitch = (CMP_DWORD)calculatePitch(source.pixelFormat, source.width);
  sourceTexture.format = toCmpFormat(source.pixelFormat);
  sourceTexture.dwDataSize = (CMP_DWORD)source.getDataSize();
  sourceTexture.pData = source.data;

  CMP_Texture destinationTexture{};
  destinationTexture.dwSize = sizeof(CMP_Texture);
  destinationTexture.dwWidth = source.width;
  destinationTexture.dwHeight = source.height;
  destinationTexture.dwPitch = source.width;
  destinationTexture.format = CMP_FORMAT_BC1;
  destinationTexture.dwDataSize = CMP_CalculateBufferSize(&destinationTexture);
  destinationTexture.pData = (CMP_BYTE*)malloc(destinationTexture.dwDataSize);

  CMP_CompressOptions options{};
  options.dwSize = sizeof(options);
  options.fquality = std::clamp(quality, 0ll, 100ll) / 100.f;
  options.dwnumThreads = 0;

  if (CMP_ConvertTexture(&sourceTexture, &destinationTexture, &options, nullptr) != CMP_OK)
  {
    return Image{};
  }
  
  Image destination;
  destination.data = destinationTexture.pData;
  destination.width = destinationTexture.dwWidth;
  destination.height = destinationTexture.dwHeight;
  destination.pixelFormat = PixelFormat::BC1;
  destination.chromaSubsampling = ChromaSubsampling_444;
  return destination;
}

// PNG ---------------------------------------------------------------------------------------------

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

// JPEG --------------------------------------------------------------------------------------------

static ChromaSubsampling tjChromaSubsamplingToDarCromaSubsampling(int tjChromaSubsampling)
{
  switch (tjChromaSubsampling)
  {
  case TJSAMP_444: return ChromaSubsampling_444;
  case TJSAMP_422: return ChromaSubsampling_422;
  case TJSAMP_420: return ChromaSubsampling_420;
  case TJSAMP_GRAY: return ChromaSubsampling_Grayscale;
  case TJSAMP_440: return ChromaSubsampling_440;
  case TJSAMP_411: return ChromaSubsampling_411;

  default:
    return ChromaSubsampling_Invalid;
  }
}

static int darChromaSubsamplingToTjSubsampling(ChromaSubsampling darChromaSubsampling)
{
  switch (darChromaSubsampling)
  {
  case ChromaSubsampling_444: return TJSAMP_444;
  case ChromaSubsampling_440: return TJSAMP_440;
  case ChromaSubsampling_422: return TJSAMP_422;
  case ChromaSubsampling_420: return TJSAMP_420;
  case ChromaSubsampling_411: return TJSAMP_411;
  case ChromaSubsampling_Grayscale: return TJSAMP_GRAY;

  default:
    return -1;
  }
}

static int darPixelFormatToTjPixelFormat(PixelFormat pixelFormat)
{
  switch (pixelFormat)
  {
  case PixelFormat::RGBA: return TJPF_RGBA;
  case PixelFormat::BGRA: return TJPF_BGRA;
  case PixelFormat::ARGB: return TJPF_ARGB;
  case PixelFormat::ABGR: return TJPF_ABGR;
  case PixelFormat::CMYK: return TJPF_CMYK;
  case PixelFormat::RGB: return TJPF_RGB;
  case PixelFormat::BGR: return TJPF_BGR;
  case PixelFormat::Grayscale: return TJPF_GRAY;

  default:
    return TJPF_UNKNOWN;
  }
}

JpegReader::JpegReader()
  : decompressor{tjInitDecompress()}
{
  if (!decompressor)
  {
    logError("Failed to initialize jpeg decompressor.");
  }
}

JpegReader::JpegReader(JpegReader&& other)
{
  using std::swap;

  swap(decompressor, other.decompressor);
}

JpegReader::~JpegReader()
{
  if (decompressor)
  {
    tjDestroy(decompressor);
  }
}

Image JpegReader::read(const wchar_t* filePath, PixelFormat outputPixelFormat)
{
  std::vector<byte> colormapJpeg;
  if (!tryReadEntireFile(filePath, colormapJpeg))
  {
    return Image();
  }

  return read(colormapJpeg.data(), colormapJpeg.size(), outputPixelFormat);
}

Image JpegReader::read(const byte* jpegData, int64 jpegDataSize, PixelFormat outputPixelFormat)
{
  if (!decompressor)
  {
    return {};
  }

  int width, height, chromaSubsampling, colorSpace;
  if (tjDecompressHeader3(decompressor, jpegData, unsigned long(jpegDataSize), &width, &height, &chromaSubsampling, &colorSpace) != 0)
  {
    logError("Failed to decompress jpeg image header.");
    return {};
  }

  Image image;
  const int64 dataSize = (width * height * toPixelSizeInBits(outputPixelFormat)) / 8;
  image.data = static_cast<byte*>(malloc(dataSize));
  image.width = width;
  image.height = height;
  image.pixelFormat = outputPixelFormat;
  image.chromaSubsampling = tjChromaSubsamplingToDarCromaSubsampling(chromaSubsampling);

  if (tjDecompress2(decompressor, jpegData, unsigned long(jpegDataSize), image.data, width, 0, height, darPixelFormatToTjPixelFormat(outputPixelFormat), TJFLAG_NOREALLOC) != 0)
  {
    logError("Failed to decompress jpeg image. %s", tjGetErrorStr2(decompressor));
    return {};
  }

  return image;
}

JpegWriter::JpegWriter()
  : compressor{ tjInitCompress() }
{
  if (!compressor)
  {
    logError("Failed to initialize jpeg compressor.");
  }
}

JpegWriter::~JpegWriter()
{
  if (compressor)
  {
    tjDestroy(compressor);
  }
}

bool JpegWriter::tryWrite(const Image& image, int8 quality, const char* fileName)
{
  if (!compressor)
  {
    return false;
  }

  byte* jpegData = nullptr;
  unsigned long jpegDataSize = 0;
  if (tjCompress2(compressor, image.data, image.width, 0, image.height, darPixelFormatToTjPixelFormat(image.pixelFormat), &jpegData, &jpegDataSize, 
      darChromaSubsamplingToTjSubsampling(image.chromaSubsampling), int(quality), TJFLAG_ACCURATEDCT) != 0)
  {
    logError("Failed to compress jpeg image. %s", tjGetErrorStr2(compressor));
    return false;
  }

  FILE* file;
  if (fopen_s(&file, fileName, "wb") != 0)
  {
    tjFree(jpegData);
    logError("Failed to open output file for jpeg write.");
    return false;
  }

  const size_t writtenBytes = fwrite(jpegData, 1, jpegDataSize, file);
  if (writtenBytes != jpegDataSize)
  {
    logError("Failed to write jpeg image to %s, %llu bytes were written.", fileName, writtenBytes);
  }

  if (fclose(file) != 0)
  {
    logError("Failed to close JPEG write file %s", fileName);
  }
  tjFree(jpegData);

  return true;
}

// WEBP --------------------------------------------------------------------------------------------

Image WebpReader::read(const byte* data, int64 dataSize, PixelFormat outputPixelFormat)
{
  int width, height;
  if (!WebPGetInfo(data, size_t(dataSize), &width, &height))
  {
    return Image();
  }

  int outputStride = (width * toPixelSizeInBits(outputPixelFormat)) / 8;

  Image image;
  image.data = (byte*)malloc(height * outputStride);
  image.width = width;
  image.height = height;
  image.pixelFormat = outputPixelFormat;
  image.chromaSubsampling = ChromaSubsampling_444;

  uint8_t* result = nullptr;
  switch (outputPixelFormat)
  {
    case PixelFormat::RGBA:
      result = WebPDecodeRGBAInto(data, size_t(dataSize), image.data, image.getDataSize(), outputStride);
      break;
    case PixelFormat::BGRA:
      result = WebPDecodeBGRAInto(data, size_t(dataSize), image.data, image.getDataSize(), outputStride);
      break;
    case PixelFormat::ARGB:
      result = WebPDecodeARGBInto(data, size_t(dataSize), image.data, image.getDataSize(), outputStride);
      break;
    case PixelFormat::RGB:
      result = WebPDecodeRGBInto(data, size_t(dataSize), image.data, image.getDataSize(), outputStride);
      break;
    case PixelFormat::BGR:
      result = WebPDecodeBGRInto(data, size_t(dataSize), image.data, image.getDataSize(), outputStride);
      break;
    default:
      logError("Cannot read WebP. Unsupported output pixel format.");
      return Image();
  }

  if (!result)
  {
    logError("Failed to decode WebP image.");
    return Image();
  }

  return image;
}

void writeAsDds(const Image& image, const char* fileName)
{
  ensureTrue(image.width > 0);
  ensureTrue(image.getDataSize() > 0);

  CMP_Texture texture{};
  texture.dwSize = sizeof(texture);
  texture.dwWidth = image.width;
  texture.dwHeight = image.height;
  texture.dwPitch = (CMP_DWORD)calculatePitch(image.pixelFormat, image.width);
  texture.format = toCmpFormat(image.pixelFormat);
  texture.dwDataSize = (CMP_DWORD)image.getDataSize();
  texture.pData = image.data;

  SaveDDSFile(fileName, texture);
}