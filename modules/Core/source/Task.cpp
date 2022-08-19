#define DAR_MODULE_NAME "Task"

#include "Core/Task.hpp"

#include <intrin.h>

TaskEventRef::TaskEventRef(TaskEvent* inTaskEvent)
  : taskEvent(inTaskEvent)
{
  if (taskEvent)
  {
    taskEvent->ref();
  }
}
TaskEventRef::TaskEventRef(const TaskEventRef& other)
  : taskEvent(other.taskEvent)
{
  if (taskEvent)
  {
    taskEvent->ref();
  }
}
TaskEventRef::TaskEventRef(TaskEventRef&& other) noexcept
{
  swap(*this, other);
}
TaskEventRef& TaskEventRef::operator=(const TaskEventRef& rhs)
{
  if (taskEvent)
  {
    taskEvent->unref();
  }

  taskEvent = rhs.taskEvent;
  if (taskEvent)
  {
    taskEvent->ref();
  }

  return *this;
}
TaskEventRef& TaskEventRef::operator=(TaskEventRef&& rhs) noexcept
{
  swap(*this, rhs);
  return *this;
}
TaskEventRef::~TaskEventRef()
{
  if (taskEvent)
  {
    taskEvent->unref();
  }
}
void swap(TaskEventRef& first, TaskEventRef& second)
{
  using std::swap;

  swap(first.taskEvent, second.taskEvent);
}

FixedThreadSafePoolAllocator<TaskEvent::SubsequentList::Node, 1024> TaskEvent::SubsequentList::nodeAllocator;
bool TaskEvent::SubsequentList::tryAdd(TaskEventRef&& taskEvent)
{
  if (isComplete)
  {
    return false;
  }

  Node* newHead = new (nodeAllocator.allocate()) Node();
  newHead->taskEvent = std::move(taskEvent);

  do 
  {
    Node* previousHead = head.load(std::memory_order_acquire);
    newHead->next = previousHead;
    if (head.compare_exchange_strong(previousHead, newHead))
    {
      return true;
    }
  } while (!isComplete);

  recycle(newHead);

  return false;
}
void TaskEvent::SubsequentList::complete()
{
  isComplete = true;

  Node* previousHead;
  do
  {
    previousHead = head.load(std::memory_order_acquire);
  } while (!head.compare_exchange_strong(previousHead, nullptr));

  while (previousHead)
  {
    previousHead->taskEvent->removePrerequisite();
    Node* toDelete = previousHead;
    previousHead = previousHead->next;
    recycle(toDelete);
  }
}
void TaskEvent::SubsequentList::recycle(Node* node)
{
  node->~Node();
  nodeAllocator.deallocate(node);
}

static FixedThreadSafePoolAllocator<TaskEvent, 512> taskEventAllocator;
TaskEventRef TaskEvent::create() 
{ 
  return TaskEventRef(new (taskEventAllocator.allocate()) TaskEvent()); 
}
TaskEventRef TaskEvent::create(TaskFunction function, void* data, ThreadType desiredThread) 
{
  return TaskEventRef(new (taskEventAllocator.allocate()) TaskEvent(function, data, desiredThread)); 
}
TaskEvent::TaskEvent(TaskFunction inFunction, void* inData, ThreadType inDesiredThread)
  : function(inFunction)
  , data(inData)
  , desiredThread(inDesiredThread)
{
}
void TaskEvent::ref()
{
  ++refCount;
}
void TaskEvent::unref()
{
  const int8 newCount = --refCount;
  assert(newCount >= 0);
  if (newCount == 0)
  {
    this->~TaskEvent();
    taskEventAllocator.deallocate(this);
  }
}
void TaskEvent::addPrerequisite()
{
  ++prerequisiteCount;
}
void TaskEvent::removePrerequisite()
{
  const int8 newCount = --prerequisiteCount;
  assert(newCount >= 0);
  if (newCount == 0)
  {
    taskManager.enqueue(function, data, desiredThread, TaskEventRef(this));
  }
}

TaskManager taskManager;

TaskManager::TaskManager()
  : parallelForFinishedEvent(CreateEvent(NULL, true, true, NULL))
{
}
TaskManager::~TaskManager()
{
  if (!isInitialized())
  {
    return;
  }

  deinitialize();
}
void TaskManager::initialize()
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
void TaskManager::initialize(int inThreadCount)
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
void TaskManager::deinitialize()
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
TaskEventRef TaskManager::schedule(TaskFunction function, void* data, ThreadType desiredThread)
{
  TaskEventRef completionEvent = TaskEvent::create(function, data, ThreadType::Main);
  enqueue(function, data, desiredThread, completionEvent);
  return completionEvent;
}
TaskEventRef TaskManager::schedule(TaskFunction function, void* data, ThreadType desiredThread, TaskEventRef* prerequisites, int8 prerequisiteCount)
{
  if (!prerequisites)
  {
    // TODO: add non fatal type of asserts for these cases.
    assert(prerequisiteCount == 0);
    return schedule(function, data, desiredThread);
  }
  assert(prerequisiteCount > 0);

  TaskEventRef completionEvent = TaskEvent::create(function, data, desiredThread);
  completionEvent->setPrerequisites(prerequisiteCount);
  for (int64 i = 0; i < prerequisiteCount; ++i)
  {
    if (!prerequisites[i]->tryAddSubsequent(completionEvent))
    {
      completionEvent->removePrerequisite();
    }
  }
  return completionEvent;
}
void TaskManager::enqueue(TaskFunction function, void* data, ThreadType desiredThread, TaskEventRef completionEvent)
{
  switch (desiredThread)
  {
    case ThreadType::Main:
      enqueueToMain(function, data, std::move(completionEvent));
      break;

    case ThreadType::Worker:
      enqueueToWorker(function, data, std::move(completionEvent));
      break;

    default:
      logError("Attempted to schedule to unknown thread.");
      assert(false);
      break;
  }
}
void TaskManager::enqueueToMain(TaskFunction function, void* data, TaskEventRef&& completionEvent)
{
  mainTaskQueue.enqueue(Task{ function, data, std::move(completionEvent) });
}
void TaskManager::enqueueToWorker(TaskFunction task, void* taskData, TaskEventRef&& completionEvent)
{
  // At first glance it could seem it's not necessary to lock the entire method, 
  // but even if we locked before the write into the queu and incrementing the taskIndexToWrite,
  // we would still have to do the previous instructions anyway.
  std::lock_guard<std::mutex> writerMutexGuard{ workerQueue.writerMutex };

  const int64 taskIndexToWrite = workerQueue.taskIndexToWrite.load(std::memory_order::memory_order_relaxed);
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
  workerQueue.tasks[workerQueue.taskIndexToWrite].completionEvent = std::move(completionEvent);

  workerQueue.taskIndexToWrite.store(nextTaskIndexToWrite, std::memory_order_release);

  ReleaseSemaphore(workerQueue.semaphore, 1, NULL);
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
  parallelForTaskInternal(taskData, threadContext.index + 1); // Calling thread is 0.

  if (--taskData.threadsRemaining == 0)
  {
    delete& taskData;
  }
}
DEFINE_TASK_END

void TaskManager::parallelFor(int64 beginValue, int64 endValue, const std::function<void(int64 iterationIndex, int64 threadIndex)>& function)
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
      enqueueToWorker(&parallelForTask, taskData, TaskEventRef());
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
void TaskManager::processMainTasks()
{
  TRACE_SCOPE();

  ThreadContext context;
  context.index = 0;

  Task task;
  while (mainTaskQueue.tryDequeue(task))
  {
    task.function(task.data, context);
    if (task.completionEvent.isValid())
    {
      task.completionEvent->complete();
    }
  }
}
DWORD TaskManager::workerThreadMain(LPVOID parameter)
{
  ThreadContext& threadContext = *static_cast<ThreadContext*>(parameter);

  {
    char threadName[64];
    sprintf_s(threadName, "TaskManagerWorker %lld", threadContext.index);
    TRACE_THREAD(threadName);
  }

  while (!taskManager.threadsShouldStop)
  {
    {
      TRACE_SCOPE("waitForWork");
      WaitForSingleObject(taskManager.workerQueue.semaphore, INFINITE);
    }

    if (taskManager.threadsShouldStop)
    {
      break;
    }

    taskManager.processAllTasks(threadContext);
  }

  return 0;
}
bool TaskManager::isInitialized() const
{
  return workerQueue.semaphore != nullptr;
}
void TaskManager::processAllTasks(const ThreadContext& threadContext)
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

    Task task = workerQueue.tasks[taskIndexToRead];
    const int64 nextTaskIndexToRead = (taskIndexToRead + 1) % arrayLength(workerQueue.tasks);
    if (workerQueue.taskIndexToRead.compare_exchange_strong(taskIndexToRead, nextTaskIndexToRead))
    {
      task.function(task.data, threadContext);
      if (task.completionEvent.isValid())
      {
        task.completionEvent->complete();
      }
    }
  }
}