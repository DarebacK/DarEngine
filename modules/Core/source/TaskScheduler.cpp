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
  std::function<void(int64)> function;
  std::atomic<int64> functionCallsDoneCount = 0;
  std::atomic<int> threadsRemainingCount;
};

DEFINE_TASK_BEGIN(parallelForTask, ParallelForTaskData)
{
  int64 currentValue = data.currentValue++;
  int64 localFunctionCallsDoneCount = 0;
  while (currentValue < data.endValue)
  {
    data.function(currentValue);
    currentValue = data.currentValue++;
    // We have to count this instead of using currentValue as some iteration might take longer than the rest.
    localFunctionCallsDoneCount = ++data.functionCallsDoneCount;
  }

  const bool thisThreadFinishedLastIteration = localFunctionCallsDoneCount == data.endValue;
  if (thisThreadFinishedLastIteration)
  {
    // TODO: Signal event
  }

  if (--data.threadsRemainingCount == 0)
  {
    delete& data;
  }
}
DEFINE_TASK_END

void TaskScheduler::parallelFor(int64 beginValue, int64 endValue, std::function<void(int64)> function)
{
  // TODO: Do 1 / threadCount part of iterations here without using the shared counters.

  ParallelForTaskData* const taskData = new ParallelForTaskData{ beginValue, endValue, std::move(function), 0, threadCount + 1 };

  parallelForTask(taskData);

  if(taskData->functionCallsDoneCount.load(std::memory_order_relaxed) < endValue - beginValue)
  {
    // TODO: Wait for event 
  }
}