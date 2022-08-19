#pragma once

#include <atomic>
#include <functional>

#include "Core/Core.hpp"
#include "Core/Concurrency.hpp"
#include "Core/Memory.hpp"

/**
 * The point of this task system is to divide work into independent tasks and distribute it
 * across threads to achieve parallelism and therefore better performance.
 */ 

class TaskEvent;
class TaskEventRef
{
public:

  TaskEventRef() = default;
  explicit TaskEventRef(TaskEvent* inTaskEvent);
  TaskEventRef(const TaskEventRef& other);
  TaskEventRef(TaskEventRef&& other) noexcept;
  TaskEventRef& operator=(const TaskEventRef& rhs);
  TaskEventRef& operator=(TaskEventRef&& rhs) noexcept;
  ~TaskEventRef();

  friend void swap(TaskEventRef& first, TaskEventRef& second);

  bool isValid() const { return taskEvent != nullptr; }

  TaskEvent* operator->() { assert(taskEvent != nullptr); return taskEvent; }

private:

  TaskEvent* taskEvent = nullptr;
};

using TaskFunction = void (*)(void* taskParameter, const ThreadContext& threadContext);

/**
 * Usually represents task completion event.
 * Has prerequisites that have to be completed before it can start executing.
 * Has subsequents that wait for its completion.
 * Managed through ref counting.
 * Aligned to cache line size to avoid false sharing.
 */ 
class alignas(CACHE_LINE_SIZE) TaskEvent
{

public:

  static TaskEventRef create();
  static TaskEventRef create(TaskFunction function, void* data, ThreadType desiredThread);

  void complete() { return subsequents.complete(); }

private:

  TaskEvent(TaskFunction function, void* data, ThreadType desiredThread);
  TaskEvent() = default;
  TaskEvent(const TaskEvent& other) = delete;
  TaskEvent(TaskEvent&& other) = delete;

  void ref();
  void unref();

  void addPrerequisite();
  void setPrerequisites(int8 prerequisites) { prerequisiteCount = prerequisites; }
  void removePrerequisite();

  bool tryAddSubsequent(TaskEventRef&& taskEvent) { return subsequents.tryAdd(std::move(taskEvent)); }
  bool tryAddSubsequent(const TaskEventRef& taskEvent) { return subsequents.tryAdd(taskEvent); }

  friend class TaskEventRef;
  friend class TaskManager;

  class SubsequentList
  {
  public:

    // Returns true if the list hasn't been completed yet, false otherwise.
    bool tryAdd(TaskEventRef&& taskEvent);
    bool tryAdd(const TaskEventRef& taskEvent) { return tryAdd(TaskEventRef(taskEvent)); }

    void complete();

  private:

    struct Node
    {
      TaskEventRef taskEvent;
      Node* next = nullptr;
    };
    static FixedThreadSafePoolAllocator<Node, 1024> nodeAllocator;
    static void recycle(Node* node);

    std::atomic<Node*> head = nullptr;
    volatile bool isComplete = false;
  };
  SubsequentList subsequents;

  // It is convenient to keep this here, before the task is ready to be executed.
  TaskFunction function = nullptr;
  void* data = nullptr;
  ThreadType desiredThread = ThreadType::Unknown;

  std::atomic<int8> refCount = 0;
  std::atomic<int8> prerequisiteCount = 0;
};

// Main class of the task system. User code will mostly interact with this exclusively.
class TaskManager
{
public:

  TaskManager();
  TaskManager(const TaskManager& other) = delete;
  TaskManager(TaskManager&& other) = delete;
  ~TaskManager();

  // Initializes with threadCount == max(processorCount - 1, 1);
  void initialize();
  void initialize(int threadCount);
  void deinitialize();

  TaskEventRef schedule(TaskFunction task, void* taskData, ThreadType desiredThread);
  TaskEventRef schedule(TaskFunction task, void* taskData, ThreadType desiredThread, TaskEventRef* prerequisites, int8 prerequisiteCount);

  // endValue means 1 past end
  void parallelFor(int64 beginValue, int64 endValue, const std::function<void(int64 iterationIndex, int64 threadIndex)>& function);

  // Process tasks meant for the main thread.
  void processMainTasks();

  int64 getWorkerCount() { return static_cast<int64>(threads.size()); }

private:

  friend class TaskEvent;
  // Schedule task that's ready for execution.
  void enqueue(TaskFunction function, void* data, ThreadType desiredThread, TaskEventRef completionEvent);
  void enqueueToMain(TaskFunction task, void* taskData, TaskEventRef&& completionEvent);
  void enqueueToWorker(TaskFunction task, void* taskData, TaskEventRef&& completionEvent);

  struct Task
  {
    TaskFunction function;
    void* data;
    TaskEventRef completionEvent;
  };

  struct TaskQueue
  {
    Task tasks[255];
    void* semaphore = nullptr;

    // Keep those shared variables on separate cache lines to avoid false sharing.
    alignas(CACHE_LINE_SIZE) std::atomic<int64> taskIndexToRead = 0;
    volatile int64 cachedTaskIndexToWrite = 0;

    alignas(CACHE_LINE_SIZE) std::atomic<int64> taskIndexToWrite = 0;
    volatile int64 cachedTaskIndexToRead = 0;
    std::mutex writerMutex;
  } workerQueue;

  MPSCStaticQueue<Task, 256> mainTaskQueue;

  static constexpr int threadCountMax = 64;
  static constexpr uint8 invalidTaskIndex = 255;

  std::vector<void*> threads;
  std::vector<ThreadContext> threadContexts;

  void* parallelForFinishedEvent;

  volatile bool threadsShouldStop = false;

private:

  static unsigned long workerThreadMain(void* parameter);

  bool isInitialized() const;

  void processAllTasks(const ThreadContext& threadContext);
};

extern TaskManager taskManager;

// RAII class for taskManager initialization and deinitialization.
class TaskManagerGuard
{
public:
  TaskManagerGuard() { taskManager.initialize(); }
  TaskManagerGuard(const TaskManagerGuard& other) = delete;
  TaskManagerGuard(TaskManagerGuard&& other) = delete;
  ~TaskManagerGuard() { taskManager.deinitialize(); }
};

#define DEFINE_TASK_BEGIN(taskName, TaskDataType) \
  void taskName (void* taskParameter, const ThreadContext& threadContext) \
    { \
      TRACE_SCOPE(); \
      TaskDataType& taskData = *static_cast<TaskDataType*>(taskParameter);

#define DEFINE_TASK_END \
    }
