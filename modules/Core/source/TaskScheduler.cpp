#include "Core/TaskScheduler.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <intrin.h>

#include "Core/Task.hpp"

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

void TaskScheduler::initialize(int inThreadCount, int threadAffinitiesOffset)
{
  inThreadCount = min(inThreadCount, threadCountMax);

  for (uint64 threadIndex = 0; threadIndex < inThreadCount; ++threadIndex)
  {
    HANDLE thread = CreateThread(NULL, 0, &workerThreadMain, this, 0, NULL);
    if (thread == NULL)
    {
      logError("Failed to create worker thread %llu", threadIndex);
      continue;
    }

    ++threadCount;

    DWORD_PTR threadAffinityMask = 1ull << (threadIndex + threadAffinitiesOffset);
    if (SetThreadAffinityMask(thread, threadAffinityMask) == 0)
    {
      logWarning("Failed to set thread affinity for worker thread %llu", threadIndex);
    }
  }
}

void TaskScheduler::schedule(TaskFunction task, void* taskData)
{
  queue.tasks[queue.taskIndexToWrite].function = task;
  queue.tasks[queue.taskIndexToWrite].data = task;

  // TODO: allow multiple producers? check against overwriting consumers?
  const int64 nextTaskIndexToWrite = (queue.taskIndexToWrite.load(std::memory_order::memory_order_relaxed) + 1) % arrayLength(queue.tasks);
  queue.taskIndexToWrite.store(nextTaskIndexToWrite, std::memory_order_release);
}

struct ParallelForTaskData
{
  std::atomic<int64> currentValue;
  int64 endValue;
  std::atomic<int> threadsRemainingCount;
  std::function<void(int64)> function;
};

DEFINE_TASK_BEGIN(parallelForTask, ParallelForTaskData)
{
  int64 currentValue = data.currentValue++;
  while (currentValue < data.endValue)
  {
    data.function(currentValue);
    currentValue = data.currentValue++;
  }

  if (--data.threadsRemainingCount == 0)
  {
    delete &data;
  }
}
DEFINE_TASK_END

void TaskScheduler::parallelFor(int64 beginValue, int64 endValue, std::function<void(int64)> function)
{
  const int threadsRemaingCount = threadCount + 1; // include this thread.
  ParallelForTaskData* taskData = new ParallelForTaskData{ beginValue, endValue, threadsRemaingCount, std::move(function) };

  parallelForTask(taskData);
}