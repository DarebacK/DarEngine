#pragma once

#include <atomic>

#include "Core/Core.hpp"

// Allocated memory aligned to alignment. Supports any alignment and size.
// This may cause more memory overhead than necessary. For smaller alignments, consider implementing small alignment versions.
void* alignedMalloc(std::size_t alignment, std::size_t size);
void alignedFree(void* pointer);

// Allocates objects from a fixed size array. Unallocated objects are managed using a free list.
template<typename ObjectType, int64 size>
class FixedThreadSafePoolAllocator
{
  static_assert(size > 0, "size must be greater than zero");
  static_assert(sizeof(ObjectType) >= sizeof(void*), "ObjectType size has to be at least size of a pointer to allow free list management");

public:

  FixedThreadSafePoolAllocator()
  {
    freeListHead.store(reinterpret_cast<FreeListItem*>(pool), std::memory_order_relaxed);
    for (int64 i = 0; i < (size - 1); ++i)
    {
      FreeListItem* nextItem = reinterpret_cast<FreeListItem*>(&pool[(i + 1) * sizeof(ObjectType)]);
      reinterpret_cast<FreeListItem*>(&pool[i * sizeof(ObjectType)])->next = nextItem;
    }
    reinterpret_cast<FreeListItem*>(&pool[(size - 1) * sizeof(ObjectType)])->next = nullptr;
  }

  void* allocate()
  {
    FreeListItem* lastFreeListHead;
    do
    {
      lastFreeListHead = freeListHead.load(std::memory_order_relaxed);
      if (!lastFreeListHead)
      {
        logWarning("FixedThreadSafePoolAllocator ran out of preallocated pool objects.");
        if (alignof(ObjectType) > alignof(std::max_align_t))
        {
          return alignedMalloc(alignof(ObjectType), sizeof(ObjectType));
        }
        else
        {
          return malloc(sizeof(ObjectType));
        }
      }
    } while (!freeListHead.compare_exchange_strong(lastFreeListHead, lastFreeListHead->next));

    return lastFreeListHead;
  }

  void deallocate(ObjectType* toDeallocate)
  {
    if (!toDeallocate)
    {
      logError("Tried to deallocate nullptr in a FixedThreadSafePoolAllocator.");
      return;
    }

    FreeListItem* newFreeListHead = reinterpret_cast<FreeListItem*>(toDeallocate);
    FreeListItem* lastFreeListHead;
    do
    {
      lastFreeListHead = freeListHead.load(std::memory_order_relaxed);
      newFreeListHead->next = lastFreeListHead;
    } while(!freeListHead.compare_exchange_strong(lastFreeListHead, newFreeListHead));
  }

private:

  byte alignas(alignof(ObjectType)) pool[size * sizeof(ObjectType)]; // Is byte array to avoid default initialization of objects.

  struct FreeListItem
  {
    FreeListItem* next;
  };
  std::atomic<FreeListItem*> alignas(CACHE_LINE_SIZE) freeListHead; // Keep on separate cache line to avoid false sharing.
};