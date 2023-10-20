#define DAR_MODULE "Concurrency"

#include "Core/Concurrency.hpp"

uint32 mainThreadId = 0;
bool isInMainThread()
{
  return GetCurrentThreadId() == mainThreadId;
}