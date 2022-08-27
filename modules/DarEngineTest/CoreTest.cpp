#include "pch.h"

#include "Core/Memory.hpp"

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