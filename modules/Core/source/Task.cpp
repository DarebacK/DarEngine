#define DAR_MODULE_NAME "Task"

#include "Core/Task.hpp"

#include <intrin.h>

TaskScheduler taskScheduler;

TaskScheduler::TaskScheduler()
  : parallelForFinishedEvent(CreateEvent(NULL, true, true, NULL))
{
}

TaskScheduler::~TaskScheduler()
{
  if (!isInitialized())
  {
    return;
  }

  deinitialize();
}

void TaskScheduler::initialize()
{
  if (isInitialized())
  {
    return;
  }

  SYSTEM_INFO systemInfo;
  GetSystemInfo(&systemInfo);
  const int workerThreadCount = std::max(int(systemInfo.dwNumberOfProcessors - 1), 1);
  initialize(workerThreadCount);
}

void TaskScheduler::initialize(int inThreadCount)
{
  if (isInitialized())
  {
    return;
  }

  inThreadCount = std::min(inThreadCount, threadCountMax);
  threads.resize(inThreadCount);
  threadContexts.resize(inThreadCount);

  workerQueue.semaphore = CreateSemaphore(NULL, 0, inThreadCount, NULL);

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
    threadContexts[threadIndex] = {static_cast<int64>(threadIndex)};

    ResumeThread(thread);
  }
}

void TaskScheduler::deinitialize()
{
  threadsShouldStop = true;

  if (workerQueue.semaphore)
  {
    while (ReleaseSemaphore(workerQueue.semaphore, 1, NULL)); // wake up worker threads so they can exit.
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
  threads.clear();
  threadContexts.clear();

  if (parallelForFinishedEvent)
  {
    CloseHandle(parallelForFinishedEvent);
    parallelForFinishedEvent = nullptr;
  }

  if (workerQueue.semaphore)
  {
    CloseHandle(workerQueue.semaphore);
    workerQueue.semaphore = nullptr;
  }
}

void TaskScheduler::scheduleToWorker(TaskFunction task, void* taskData)
{
  const int64 taskIndexToWrite = workerQueue.taskIndexToWrite.load(std::memory_order::memory_order_relaxed);
  // TODO: allow multiple producers?
  const int64 nextTaskIndexToWrite = (taskIndexToWrite + 1) % arrayLength(workerQueue.tasks);

  if (nextTaskIndexToWrite == workerQueue.cachedTaskIndexToRead)
  {
    workerQueue.cachedTaskIndexToRead = workerQueue.taskIndexToRead.load(std::memory_order::memory_order_relaxed);
    if (nextTaskIndexToWrite == workerQueue.cachedTaskIndexToRead)
    {
      int spinCount = 0;
      constexpr int maxSpinCount = 1000;
      do
      {
        workerQueue.cachedTaskIndexToRead = workerQueue.taskIndexToRead.load(std::memory_order::memory_order_relaxed);

        logWarning("Cannot write new task with index %lld as the queue is full.", taskIndexToWrite);
      } while (nextTaskIndexToWrite == workerQueue.cachedTaskIndexToRead && spinCount++ < maxSpinCount);
    }
  }

  workerQueue.tasks[workerQueue.taskIndexToWrite].function = task;
  workerQueue.tasks[workerQueue.taskIndexToWrite].data = taskData;

  workerQueue.taskIndexToWrite.store(nextTaskIndexToWrite, std::memory_order_release);

  ReleaseSemaphore(workerQueue.semaphore, 1, NULL);
}

void TaskScheduler::scheduleToMain(TaskFunction task, void* taskData)
{
  mainTaskQueue.enqueue(Task{ task, taskData });
}

struct ParallelForTaskData
{
  std::atomic<int64> currentValue;
  int64 endValue;
  std::function<void(int64 iterationIndex, int64 threadIndex)> function;
  std::atomic<int64> functionCallsDoneCount;
  std::atomic<int64> threadsRemaining;
  void* finishedEvent;
};

static void parallelForTaskInternal(ParallelForTaskData& taskData, int64 threadIndex)
{
  int64 currentValue = taskData.currentValue++;
  int64 localFunctionCallsDoneCount = 0;
  while (currentValue < taskData.endValue)
  {
    taskData.function(currentValue, threadIndex);
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
  parallelForTaskInternal(taskData, threadContext.threadIndex + 1); // Calling thread is 0.

  if (--taskData.threadsRemaining == 0)
  {
    delete& taskData;
  }
}
DEFINE_TASK_END

void TaskScheduler::parallelFor(int64 beginValue, int64 endValue, const std::function<void(int64 iterationIndex, int64 threadIndex)>& function)
{
  TRACE_SCOPE();

  DWORD waitResult = WaitForSingleObject(parallelForFinishedEvent, 0);
  switch (waitResult)
  {
    case WAIT_TIMEOUT:
      logError("parallelForFinishedEvent is not signaled from last call. It will be leaked.");
      [[fallthrough]];
    case WAIT_FAILED:
      logError("Failed to wait on parallelForFinishedEvent from last call. It will be leaked.");
      parallelForFinishedEvent = CreateEvent(NULL, true, true, NULL); // old one will leak, but this situation shouldn't happen.
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
    const int64 totalThreadCount = int64(threads.size() + 1); // We assume the calling thread is not one of the worker threads.
    taskData = new ParallelForTaskData{ beginValueForWorkerThreads, endValue, function, iterationCountToDoInCurrentThread, totalThreadCount, parallelForFinishedEvent };
    for(size_t i = 0; i < threads.size(); ++i)
    {
      scheduleToWorker(&parallelForTask, taskData);
    }
  }
  else
  {
    SetEvent(parallelForFinishedEvent);
  }

  for (int64 i = beginValue; i < beginValue + iterationCountToDoInCurrentThread; ++i)
  {
    function(i, 0);
  }

  if (taskData)
  {
    const int64 iterationCountToDoInWorkerThreads = endValue - beginValueForWorkerThreads;
    const int64 iterationsDoneInWorkerThreads = taskData->functionCallsDoneCount.load(std::memory_order_relaxed) - iterationCountToDoInCurrentThread;
    if (iterationsDoneInWorkerThreads < iterationCountToDoInWorkerThreads)
    {
      parallelForTaskInternal(*taskData, 0); // Help out with remaining iterations.
    }

    if (--taskData->threadsRemaining == 0)
    {
      delete taskData;
    }
    else
    {
      constexpr DWORD waitTimeoutMs = 1000;
      waitResult = WaitForSingleObject(parallelForFinishedEvent, waitTimeoutMs);
      switch (waitResult)
      {
        case WAIT_TIMEOUT:
          logError("parallelForFinishedEvent timeout after %d ms.", waitTimeoutMs);
          break;

        case WAIT_FAILED:
          logError("Failed to wait on parallelForFinishedEvent.");
          break;

        default:
          break;
      }
    }
  }
}

void TaskScheduler::processMainTasks()
{
  ThreadContext context;
  context.threadIndex = 0;

  Task task;
  while (mainTaskQueue.tryDequeue(task))
  {
    task.function(task.data, context);
  }
}

DWORD TaskScheduler::workerThreadMain(LPVOID parameter)
{
  TaskScheduler::ThreadContext& threadContext = *static_cast<TaskScheduler::ThreadContext*>(parameter);

  {
    char threadName[64];
    sprintf_s(threadName, "TaskSchedulerWorker %lld", threadContext.threadIndex);
    TRACE_THREAD(threadName);
  }

  while (!taskScheduler.threadsShouldStop)
  {
    {
      TRACE_SCOPE("waitForWork");
      WaitForSingleObject(taskScheduler.workerQueue.semaphore, INFINITE);
    }

    if (taskScheduler.threadsShouldStop)
    {
      break;
    }

    taskScheduler.processAllTasks(threadContext);
  }

  return 0;
}

bool TaskScheduler::isInitialized() const
{
  return workerQueue.semaphore != nullptr;
}

void TaskScheduler::processAllTasks(const ThreadContext& threadContext)
{
  while (true)
  {
    int64 taskIndexToRead = workerQueue.taskIndexToRead.load(std::memory_order::memory_order_relaxed);

    if (taskIndexToRead == workerQueue.cachedTaskIndexToWrite)
    {
      workerQueue.cachedTaskIndexToWrite = workerQueue.taskIndexToWrite.load(std::memory_order::memory_order_acquire);
      if (taskIndexToRead == workerQueue.cachedTaskIndexToWrite)
      {
        return;
      }
    }

    const Task task = workerQueue.tasks[taskIndexToRead];
    const int64 nextTaskIndexToRead = (taskIndexToRead + 1) % arrayLength(workerQueue.tasks);
    if (workerQueue.taskIndexToRead.compare_exchange_strong(taskIndexToRead, nextTaskIndexToRead))
    {
      task.function(task.data, threadContext);
    }
  }
}