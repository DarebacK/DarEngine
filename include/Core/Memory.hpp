#pragma once

#include <atomic>

#include "Core/Core.hpp"

// Allocated memory aligned to alignment. Supports any alignment and size.
// This may cause more memory overhead than necessary. For smaller alignments, consider implementing small alignment versions.
void* alignedMalloc(std::size_t alignment, std::size_t size);
void alignedFree(void* pointer);
inline bool isAligned(void* ptr, size_t alignment) { return uintptr_t(ptr) % alignment == 0; }

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

  alignas(alignof(ObjectType)) byte pool[size * sizeof(ObjectType)]; // Is byte array to avoid default initialization of objects.

  struct FreeListItem
  {
    FreeListItem* next;
  };
  alignas(CACHE_LINE_SIZE) std::atomic<FreeListItem*> freeListHead; // Keep on separate cache line to avoid false sharing.
};

// Value type must implement ref() and unref() methods.
template<typename ValueType>
class Ref
{
public:

  Ref() = default;
  explicit Ref(ValueType* InPtr)
    : ptr(InPtr)
  {
    if (ptr)
    {
      ptr->ref();
    }
  }
  Ref(const Ref& other)
    : ptr(other.ptr)
  {
    if (ptr)
    {
      ptr->ref();
    }
  }
  Ref(Ref&& other) noexcept
  {
    swap(*this, other);
  }
  Ref& operator=(const Ref& rhs)
  {
    if (ptr)
    {
      ptr->unref();
    }

    ptr = rhs.ptr;
    if (ptr)
    {
      ptr->ref();
    }

    return *this;
  }
  Ref& operator=(Ref&& rhs) noexcept
  {
    swap(*this, rhs);
    return *this;
  }
  ~Ref()
  {
    if (ptr)
    {
      ptr->unref();
    }
  }

  friend void swap(Ref& first, Ref& second)
  {
    using std::swap;

    swap(first.ptr, second.ptr);
  }

  bool isValid() const { return ptr != nullptr; }

  ValueType* operator->() { assert(ptr != nullptr); return ptr; }

private:

  ValueType* ptr = nullptr;
};
