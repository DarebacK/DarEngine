#include "Core/TaskScheduler.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

TaskScheduler::~TaskScheduler()
{
  // TODO
}

static DWORD workerThreadMain(LPVOID parameter)
{
  TaskScheduler* scheduler = static_cast<TaskScheduler*>(parameter);

  logInfo("Hello worker thread %lu on %lu core", GetCurrentThreadId(), GetCurrentProcessorNumber());

  return 0;
}

void TaskScheduler::initialize(int threadCount, int threadAffinitiesOffset)
{
  for (uint64 threadIndex = 0; threadIndex < threadCount; ++threadIndex)
  {
    HANDLE thread = CreateThread(NULL, 0, &workerThreadMain, this, 0, NULL);
    if (thread == NULL)
    {
      logError("Failed to create worker thread %llu", threadIndex);
      continue;
    }

    DWORD_PTR threadAffinityMask = 1ull << (threadIndex + threadAffinitiesOffset);
    if (SetThreadAffinityMask(thread, threadAffinityMask) == 0)
    {
      logWarning("Failed to set thread affinity for worker thread %llu", threadIndex);
    }
  }
}

void TaskScheduler::schedule(TaskFunction task, void* taskData)
{

}