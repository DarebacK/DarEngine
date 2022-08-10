#pragma once

#include <atomic>
#include <functional>

#include "Core/Core.hpp"

// Schedules tasks to its worker threads. Each thread is assigned its affinity to one core.
class TaskScheduler
{
public:

  struct ThreadContext
  {
    TaskScheduler* taskScheduler;
    int64 threadIndex;
  };
  using TaskFunction = void (*)(void* taskParameter, const ThreadContext& threadContext);

  TaskScheduler();
  TaskScheduler(const TaskScheduler& other) = delete;
  TaskScheduler(TaskScheduler&& other) = delete;
  ~TaskScheduler();

  // Initializes with threadCount == max(processorCount - 1, 1);
  void initialize();
  void initialize(int threadCount);
  void deinitialize();

  void schedule(TaskFunction task, void* taskData);

  // endValue means 1 past end
  void parallelFor(int64 beginValue, int64 endValue, const std::function<void(int64 iterationIndex, int64 threadIndex)>& function);

  int64 getThreadCount() { return static_cast<int64>(threads.size()); }

private:

  struct Task
  {
    TaskFunction function;
    void* data;
  };

  // TODO: Maybe separate to a templated independent SPMC queue?
  struct TaskQueue
  {
    // Only 255 and not 256 because index 255 is reserved for invalid state.
    Task tasks[255];
    void* semaphore = nullptr;
    byte padding1[8];

    // Keep those shared variables on separate cache lines to avoid false sharing.
    alignas(CACHE_LINE_SIZE) std::atomic<int64> taskIndexToRead = 0;
    volatile int64 cachedTaskIndexToWrite = 0;
    byte padding2[CACHE_LINE_SIZE - 2*sizeof(int64)];

    alignas(CACHE_LINE_SIZE) std::atomic<int64> taskIndexToWrite = 0;
    volatile int64 cachedTaskIndexToRead = 0;
    byte padding3[CACHE_LINE_SIZE - 2*sizeof(int64)];
  } queue;

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

extern TaskScheduler taskScheduler;

class TaskSchedulerGuard
{
public:
  TaskSchedulerGuard() { taskScheduler.initialize(); }
  TaskSchedulerGuard(const TaskSchedulerGuard& other) = delete;
  TaskSchedulerGuard(TaskSchedulerGuard&& other) = delete;
  ~TaskSchedulerGuard() { taskScheduler.deinitialize(); }
};

#define DEFINE_TASK_BEGIN(taskName, TaskDataType) \
  void taskName (void* taskParameter, const TaskScheduler::ThreadContext& threadContext) \
    { \
      TRACE_SCOPE(); \
      TaskDataType& taskData = *static_cast<TaskDataType*>(taskParameter);

#define DEFINE_TASK_END \
    }
