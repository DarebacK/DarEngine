#define DAR_MODULE_NAME "Task"

#include "Core/Task.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <intrin.h>

TaskScheduler::TaskScheduler()
  : parallelForFinishedEvent(CreateEvent(NULL, true, true, NULL))
{
}

TaskScheduler::~TaskScheduler()
{
  threadsShouldStop = true;

  if (threadSemaphore)
  {
    while (ReleaseSemaphore(threadSemaphore, 1, NULL)); // wake up worker threads so they can exit.
  }

  for (int threadIndex = 0; threadIndex < threads.size(); ++threadIndex)
  {
    void* thread = threads[threadIndex];
    if (!thread)
    {
      continue;
    }

    constexpr DWORD waitTimeoutMs = 1000;
    DWORD waitResult = WaitForSingleObject(thread, waitTimeoutMs);
    switch (waitResult)
    {
    case WAIT_TIMEOUT:
      logError("Thread %d index %d stop timeout %d ms.", GetThreadId(thread), threadIndex, waitTimeoutMs);
      break;

    case WAIT_FAILED:
      logError("Thread %d index %d stop failed.", GetThreadId(thread), threadIndex);
      break;

    default:
      CloseHandle(thread);
      break;
    }
  }

  if (parallelForFinishedEvent)
  {
    CloseHandle(parallelForFinishedEvent);
  }

  if (threadSemaphore)
  {
    CloseHandle(threadSemaphore);
  }
}

void TaskScheduler::initialize(int inThreadCount, int threadAffinitiesOffset)
{
  inThreadCount = min(inThreadCount, threadCountMax);
  threads.resize(inThreadCount);
  threadContexts.resize(inThreadCount);

  threadSemaphore = CreateSemaphore(NULL, 0, inThreadCount, NULL);

  for (uint64 threadIndex = 0; threadIndex < inThreadCount; ++threadIndex)
  {
    HANDLE thread = CreateThread(NULL, 0, &workerThreadMain, &threadContexts[threadIndex], CREATE_SUSPENDED, NULL);
    if (thread == NULL)
    {
      threads[threadIndex] = {};
      threadContexts[threadIndex] = {};
      logError("Failed to create worker thread %llu", threadIndex);
      continue;
    }

    threads[threadIndex] = thread;
    threadContexts[threadIndex] = {this, static_cast<int64>(threadIndex)};

    DWORD_PTR threadAffinityMask = 1ull << (threadIndex + threadAffinitiesOffset);
    if (SetThreadAffinityMask(thread, threadAffinityMask) == 0)
    {
      logWarning("Failed to set thread affinity for worker thread %llu", threadIndex);
    }

    ResumeThread(thread);
  }
}

void TaskScheduler::schedule(TaskFunction task, void* taskData)
{
  queue.tasks[queue.taskIndexToWrite].function = task;
  queue.tasks[queue.taskIndexToWrite].data = task;

  // TODO: allow multiple producers?
  const int64 nextTaskIndexToWrite = (queue.taskIndexToWrite.load(std::memory_order::memory_order_relaxed) + 1) % arrayLength(queue.tasks);
  int spinCount = 0;
  while (taskIsBeingConsumed(nextTaskIndexToWrite) && spinCount++ < 100)
  {
    logWarning("Cannot write new task with index %lld as it's still being consumed.", nextTaskIndexToWrite);
  }
  queue.taskIndexToWrite.store(nextTaskIndexToWrite, std::memory_order_release);

  ReleaseSemaphore(threadSemaphore, 1, NULL);
}

struct ParallelForTaskData
{
  std::atomic<int64> currentValue;
  int64 endValue;
  std::function<void(int64)> function;
  std::atomic<int64> functionCallsDoneCount;
  void* finishedEvent;
  std::atomic<int64> workerThreadsRemainingCount;
};

static void parallelForTaskInternal(ParallelForTaskData& taskData)
{
  int64 currentValue = taskData.currentValue++;
  int64 localFunctionCallsDoneCount = 0;
  while (currentValue < taskData.endValue)
  {
    taskData.function(currentValue);
    currentValue = taskData.currentValue++;
    // We have to count this instead of using currentValue as some iteration might take longer than the rest.
    localFunctionCallsDoneCount = ++taskData.functionCallsDoneCount;
  }

  const bool thisThreadFinishedLastIteration = localFunctionCallsDoneCount == taskData.endValue;
  if (thisThreadFinishedLastIteration)
  {
    SetEvent(taskData.finishedEvent);
  }
}

DEFINE_TASK_BEGIN(parallelForTask, ParallelForTaskData)
{
  parallelForTaskInternal(taskData);

  if (--taskData.workerThreadsRemainingCount == 0)
  {
    delete &taskData;
  }
}
DEFINE_TASK_END

void TaskScheduler::parallelFor(int64 beginValue, int64 endValue, const std::function<void(int64)>& function)
{
  DWORD waitResult = WaitForSingleObject(parallelForFinishedEvent, 0);
  switch (waitResult)
  {
    case WAIT_TIMEOUT:
      logError("parallelForFinishedEvent is not signaled from last call.");
      parallelForFinishedEvent = CreateEvent(NULL, true, true, NULL); // old one will leak, but this situation shouldn't happen.
      break;

    case WAIT_FAILED:
      logError("failed to wait on parallelForFinishedEvent from last call.");
      break;
  }
  ResetEvent(parallelForFinishedEvent);

  const int64 totalIterationCount = endValue - beginValue;
  const int64 totalThreadCount = (threads.size() + 1);
  int64 iterationCountToDoInCurrentThread = totalIterationCount / totalThreadCount;
  if (totalIterationCount % totalThreadCount > 0)
  {
    // Do possibly a little bit more work in current thread to minimize chance of waiting for worker threads.
    ++iterationCountToDoInCurrentThread;
  }
  const int64 beginValueForWorkerThreads = beginValue + iterationCountToDoInCurrentThread;
  ParallelForTaskData* taskData = nullptr;
  if (beginValueForWorkerThreads < endValue)
  {
    taskData = new ParallelForTaskData{ beginValueForWorkerThreads, endValue, function, 0, parallelForFinishedEvent, static_cast<int64>(threads.size()) };
    for(size_t i = 0; i < threads.size(); ++i)
    {
      schedule(&parallelForTask, taskData);
    }
  }

  for (int64 i = beginValue; i < beginValue + iterationCountToDoInCurrentThread; ++i)
  {
    function(i);
  }

  if (taskData)
  {
    const int64 iterationCountToDoInWorkerThreads = endValue - beginValueForWorkerThreads;
    if(taskData->functionCallsDoneCount.load(std::memory_order_relaxed) < iterationCountToDoInWorkerThreads)
    {
      parallelForTaskInternal(*taskData); // Help out with remaining iterations.

      constexpr DWORD waitTimeoutMs = 1000;
      waitResult = WaitForSingleObject(parallelForFinishedEvent, waitTimeoutMs);
      switch (waitResult)
      {
        case WAIT_TIMEOUT:
          logError("parallelForFinishedEvent timeout after %d ms.", waitTimeoutMs);
          break;

        case WAIT_FAILED:
          logError("failed to wait on parallelForFinishedEvent.");
          break;
      }
    }
  }

}

DWORD TaskScheduler::workerThreadMain(LPVOID parameter)
{
  TaskScheduler::ThreadContext* threadContext = static_cast<TaskScheduler::ThreadContext*>(parameter);

  logInfo("Hello worker thread %lu on %lu core", GetCurrentThreadId(), GetCurrentProcessorNumber());

  while (!threadContext->taskScheduler->threadsShouldStop)
  {
    WaitForSingleObject(threadContext->taskScheduler->threadSemaphore, INFINITE);

    // TODO: Do tasks
  }

  return 0;
}

bool TaskScheduler::taskIsBeingConsumed(int64 taskIndex) const
{
  for (size_t threadIndex = 0; threadIndex < threads.size(); ++threadIndex)
  {
    if (threadCurrentTaskIndices[threadIndex] == taskIndex)
    {
      return true;
    }
  }

  return false;
}