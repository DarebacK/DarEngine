#include "pch.h"

#include "Core/Memory.hpp"
#include "Core/Config.hpp"

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