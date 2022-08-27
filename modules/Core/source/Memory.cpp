#define DAR_MODULE_NAME "Memory"

#include "Core/Memory.hpp"
#include "Core/Math.hpp"

void* alignedMalloc(std::size_t alignment, std::size_t size)
{
  const std::size_t alignmentSize = std::max((int64)alignment - (int64)sizeof(std::max_align_t), 0ll);
  void* originalPointer = malloc(size + sizeof(uintptr_t) + alignmentSize);
  void* resultPointer = reinterpret_cast<uintptr_t*>(originalPointer) + 1;
  const std::size_t shift = (alignment - reinterpret_cast<std::size_t>(resultPointer) % alignment) % alignment;
  resultPointer = reinterpret_cast<void*>(reinterpret_cast<std::size_t>(resultPointer) + shift);
  *(reinterpret_cast<uintptr_t*>(resultPointer) - 1) = reinterpret_cast<uintptr_t>(originalPointer);
  return resultPointer;
}
void alignedFree(void* pointer)
{
  assert(pointer != nullptr);
  free(reinterpret_cast<void*>(*(reinterpret_cast<uintptr_t*>(pointer) - 1)));
}