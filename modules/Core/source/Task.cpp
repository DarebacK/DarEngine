#define DAR_MODULE_NAME "Task"

#include "Core/Task.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <intrin.h>

TaskScheduler::TaskScheduler()
  : parallelForFinishedEvent(CreateEvent(NULL, true, true, NULL))
{
  for (int i = 0; i < arrayLength(threadCurrentTaskIndices); ++i)
  {
    threadCurrentTaskIndices[i] = invalidTaskIndex;
  }
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

void TaskScheduler::initialize()
{
  SYSTEM_INFO systemInfo;
  GetSystemInfo(&systemInfo);
  const int workerThreadCount = max(systemInfo.dwNumberOfProcessors - 1, 1);
  initialize(workerThreadCount, 1);
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
  const int64 taskIndexToWrite = queue.taskIndexToWrite.load(std::memory_order::memory_order_relaxed);
  // TODO: allow multiple producers?
  const int64 nextTaskIndexToWrite = (taskIndexToWrite + 1) % arrayLength(queue.tasks);

  int spinCount = 0;
  constexpr int maxSpinCount = 100;
  while (nextTaskIndexToWrite == queue.taskIndexToRead.load(std::memory_order::memory_order_relaxed) && spinCount++ < maxSpinCount)
  {
    logWarning("Cannot write new task with index %lld as the queue is full.", taskIndexToWrite);
  }

  spinCount = 0;
  while (taskIsBeingConsumed(taskIndexToWrite) && spinCount++ < maxSpinCount)
  {
    logWarning("Cannot write new task with index %lld as it's still being consumed.", taskIndexToWrite);
  }

  queue.tasks[queue.taskIndexToWrite].function = task;
  queue.tasks[queue.taskIndexToWrite].data = taskData;

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
    taskData = new ParallelForTaskData{ beginValueForWorkerThreads, endValue, function, iterationCountToDoInCurrentThread, parallelForFinishedEvent, static_cast<int64>(threads.size()) };
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
  TaskScheduler& taskScheduler = *threadContext->taskScheduler;
  const int64 threadIndex = threadContext->threadIndex;

  while (!taskScheduler.threadsShouldStop)
  {
    WaitForSingleObject(taskScheduler.threadSemaphore, INFINITE);

    if (taskScheduler.threadsShouldStop)
    {
      break;
    }

    taskScheduler.processAllTasks(threadIndex);
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

void TaskScheduler::processAllTasks(int64 threadIndex)
{
  while (true)
  {
    int64 taskIndexToRead = queue.taskIndexToRead.load(std::memory_order::memory_order_relaxed);
    const int64 taskIndexToWrite = queue.taskIndexToWrite.load(std::memory_order::memory_order_relaxed);

    if (taskIndexToRead == taskIndexToWrite)
    {
      return;
    }

    const int64 nextTaskIndexToRead = (taskIndexToRead + 1) % arrayLength(queue.tasks);
    if (queue.taskIndexToRead.compare_exchange_strong(taskIndexToRead, nextTaskIndexToRead))
    {
      threadCurrentTaskIndices[threadIndex] = static_cast<uint8>(taskIndexToRead);

      const Task task = queue.tasks[taskIndexToRead];
      task.function(task.data);

      threadCurrentTaskIndices[threadIndex] = invalidTaskIndex;
    }
  }
}