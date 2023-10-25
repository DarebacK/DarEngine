#include "pch.h"

#include "Core/Memory.hpp"
#include "Core/Config.hpp"
#include "Core/Math.hpp"
#include "Core/String.hpp"

// Memory tests ************************************************************************************

#define testAlignedMalloc(alignment, size) \
{ \
  void* ptr = alignedMalloc(alignment, size); \
  EXPECT_TRUE(ptr != nullptr); \
  EXPECT_TRUE(isAligned(ptr, alignment)); \
  alignedFree(ptr); \
} \

TEST(Memory, alignedMalloc) {

  for (int i = 1; i <= 1000; i++)
  {
    testAlignedMalloc(1, i);
    testAlignedMalloc(2, i);
    testAlignedMalloc(4, i);
    testAlignedMalloc(8, i);
    testAlignedMalloc(16, i);
    testAlignedMalloc(32, i);
    testAlignedMalloc(64, i);
    testAlignedMalloc(128, i);
    testAlignedMalloc(256, i);
    testAlignedMalloc(512, i);
    testAlignedMalloc(1024, i);
    testAlignedMalloc(2048, i);
    testAlignedMalloc(4096, i);
  }
}

// Config tests ************************************************************************************

TEST(Config, tryParseConfigSimpleValid)
{
  std::string config = "widthInMeters=1565430\nheightInMeters = 1878516 ";
  int valueCounter = 0;
  EXPECT_TRUE(tryParseConfig(config.data(), config.length(), 
    [&valueCounter](const ConfigKeyValueNode& node) -> bool
    {
      if (valueCounter == 0)
      {
        EXPECT_TRUE(strcmp(node.key, "widthInMeters") == 0);
        EXPECT_EQ(node.keyLength, strlen("widthInMeters"));
        EXPECT_EQ(node.toInt(), 1565430);
      }
      else if (valueCounter == 1)
      {
        EXPECT_TRUE(strcmp(node.key, "heightInMeters") == 0);
        EXPECT_EQ(node.keyLength, strlen("heightInMeters"));
        EXPECT_EQ(node.toInt(), 1878516);
      }

      valueCounter++;

      return false;
    }
  ));
  EXPECT_EQ(valueCounter, 2);
}

// Math tests **************************************************************************************

TEST(Math, Mat4fMultiplicationMat4f)
{
    Mat4f a{{
        { 1.f,  2.f,  3.f,  4.f},
        { 5.f,  6.f,  7.f,  8.f},
        { 9.f, 10.f, 11.f, 12.f},
        {13.f, 14.f, 15.f, 16.f}
    }};
    Mat4f b{{ 
        { 17.f, 18.f, 19.f, 20.f},
        { 21.f, 22.f, 23.f, 24.f},
        { 25.f, 26.f, 27.f, 28.f},
        { 29.f, 30.f, 31.f, 32.f}
    }};

    Mat4f c = a * b;

    EXPECT_EQ(c[0][0], 250.f);
    EXPECT_EQ(c[0][1], 260.f);
    EXPECT_EQ(c[0][2], 270.f);
    EXPECT_EQ(c[0][3], 280.f);
    EXPECT_EQ(c[1][0], 618.f);
    EXPECT_EQ(c[1][1], 644.f);
    EXPECT_EQ(c[1][2], 670.f);
    EXPECT_EQ(c[1][3], 696.f);
    EXPECT_EQ(c[2][0], 986.f);
    EXPECT_EQ(c[2][1], 1028.f);
    EXPECT_EQ(c[2][2], 1070.f);
    EXPECT_EQ(c[2][3], 1112.f);
    EXPECT_EQ(c[3][0], 1354.f);
    EXPECT_EQ(c[3][1], 1412.f);
    EXPECT_EQ(c[3][2], 1470.f);
    EXPECT_EQ(c[3][3], 1528.f);
}
TEST(Math, Mat4fMultiplicationMat4x3f)
{
    Mat4f a{ {
        { 1.f,  2.f,  3.f,  4.f},
        { 5.f,  6.f,  7.f,  8.f},
        { 9.f, 10.f, 11.f, 12.f},
        {13.f, 14.f, 15.f, 16.f}
    } };
    Mat4x3f b{ {
        { 17.f, 18.f, 19.f},
        { 20.f, 21.f, 22.f},
        { 23.f, 24.f, 25.f},
        { 26.f, 27.f, 28.f}
    } };

    Mat4f c = a * b;

    EXPECT_EQ(c[0][0], 230.f);
    EXPECT_EQ(c[0][1], 240.f);
    EXPECT_EQ(c[0][2], 250.f);
    EXPECT_EQ(c[0][3], 4.f);
    EXPECT_EQ(c[1][0], 574.f);
    EXPECT_EQ(c[1][1], 600.f);
    EXPECT_EQ(c[1][2], 626.f);
    EXPECT_EQ(c[1][3], 8.f);
    EXPECT_EQ(c[2][0], 918.f);
    EXPECT_EQ(c[2][1], 960.f);
    EXPECT_EQ(c[2][2], 1002.f);
    EXPECT_EQ(c[2][3], 12.f);
    EXPECT_EQ(c[3][0], 1262.f);
    EXPECT_EQ(c[3][1], 1320.f);
    EXPECT_EQ(c[3][2], 1378.f);
    EXPECT_EQ(c[3][3], 16.f);
}

// String tests ************************************************************************************
TEST(String, getLengthWithoutTrailingSlashes)
{
  const wchar_t* str1 = L"";
  EXPECT_EQ(getLengthWithoutTrailingSlashes(str1), 0);

  const wchar_t* str2 = L"\\";
  EXPECT_EQ(getLengthWithoutTrailingSlashes(str2), 0);

  const wchar_t* str3 = L"ab\\cd";
  EXPECT_EQ(getLengthWithoutTrailingSlashes(str3), 5);

  const wchar_t* str4 = L"ab\\cde\\";
  EXPECT_EQ(getLengthWithoutTrailingSlashes(str4), 6);

  const wchar_t* str5 = L"abc\\de/";
  EXPECT_EQ(getLengthWithoutTrailingSlashes(str5), 6);

  const wchar_t* str6 = L"abcde/fgh/";
  EXPECT_EQ(getLengthWithoutTrailingSlashes(str6), 9);
}
TEST(String, getLengthUntilFirstSlash)
{
  const wchar_t* str1 = L"";
  EXPECT_EQ(getLengthUntilFirstSlash(str1), 0);

  const wchar_t* str2 = L"\\";
  EXPECT_EQ(getLengthUntilFirstSlash(str2), 0);

  const wchar_t* str3 = L"ab\\cd";
  EXPECT_EQ(getLengthUntilFirstSlash(str3), 2);

  const wchar_t* str4 = L"ab\\cde\\";
  EXPECT_EQ(getLengthUntilFirstSlash(str4), 2);

  const wchar_t* str5 = L"abc\\de/";
  EXPECT_EQ(getLengthUntilFirstSlash(str5), 3);

  const wchar_t* str6 = L"abcde/fgh/";
  EXPECT_EQ(getLengthUntilFirstSlash(str6), 5);

  const wchar_t* str7 = L"abc";
  EXPECT_EQ(getLengthUntilFirstSlash(str7), 3);
}