#define DAR_MODULE_NAME "Memory"

#include "Core/Memory.hpp"
#include "Core/Math.hpp"

// TODO: unit test this
void* alignedMalloc(std::size_t alignment, std::size_t size)
{
  const std::size_t alignmentSize = std::max(alignment - sizeof(std::max_align_t), 0ull);
  void* originalPointer = malloc(size + sizeof(uintptr_t) + alignmentSize);
  void* resultPointer = reinterpret_cast<uintptr_t*>(originalPointer) + 1;
  const std::size_t shift = reinterpret_cast<std::size_t>(resultPointer) % alignment;
  resultPointer = reinterpret_cast<void*>(reinterpret_cast<std::size_t>(resultPointer) + shift);
  *(reinterpret_cast<uintptr_t*>(resultPointer) - 1) = reinterpret_cast<uintptr_t>(originalPointer);
  return resultPointer;
}
void alignedFree(void* pointer)
{
  free(reinterpret_cast<void*>(*(reinterpret_cast<uintptr_t*>(pointer) - 1)));
}