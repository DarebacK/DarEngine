#pragma once

#include <mutex>

#include "Core/Core.hpp"

// Multiple producers, single consumer queue with fixed size.
template<typename ItemType, uint64 queueSize>
class MPSCStaticQueue
{
public:

  MPSCStaticQueue() = default;
  MPSCStaticQueue(const MPSCStaticQueue& other) = delete;
  MPSCStaticQueue(MPSCStaticQueue&& other) = delete;
  ~MPSCStaticQueue() = default;

  void enqueue(ItemType&& item)
  {
    TRACE_SCOPE();
    {
      std::lock_guard<std::mutex> lock{ producerMutex };

      const int64 nextIndexToWrite = (indexToWrite + 1) % queueSize;
      while (nextIndexToWrite == cachedIndexToRead)
      {
        cachedIndexToRead = indexToRead;
      }
      items[indexToWrite] = std::move(item);
      indexToWrite = nextIndexToWrite;
    }
  }

  bool tryDequeue(ItemType& outItem)
  {
    if (indexToRead == cachedIndexToWrite)
    {
      cachedIndexToWrite = indexToWrite;
      if (indexToRead == cachedIndexToWrite)
      {
        return false;
      }
    }

    outItem = std::move(items[indexToRead]);
    indexToRead++;

    return true;
  }

private:

  ItemType items[queueSize];

  alignas(CACHE_LINE_SIZE) volatile int64 indexToWrite = 0;
  volatile int64 cachedIndexToRead = 0;
  std::mutex producerMutex;

  alignas(CACHE_LINE_SIZE) volatile int64 indexToRead = 0;
  int64 cachedIndexToWrite = 0;

};