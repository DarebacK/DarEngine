#pragma once

#include <atomic>
#include <functional>

#include "Core/Core.hpp"
#include "Core/Concurrency.hpp"

// Schedules tasks to its worker threads. Each thread is assigned its affinity to one core.
class TaskScheduler
{
public:

  TaskScheduler();
  TaskScheduler(const TaskScheduler& other) = delete;
  TaskScheduler(TaskScheduler&& other) = delete;
  ~TaskScheduler();

  // Initializes with threadCount == max(processorCount - 1, 1);
  void initialize();
  void initialize(int threadCount);
  void deinitialize();

  struct ThreadContext
  {
    int64 threadIndex;
  };
  using TaskFunction = void (*)(void* taskParameter, const ThreadContext& threadContext);
  void scheduleToWorker(TaskFunction task, void* taskData);
  void scheduleToMain(TaskFunction task, void* taskData);

  // endValue means 1 past end
  void parallelFor(int64 beginValue, int64 endValue, const std::function<void(int64 iterationIndex, int64 threadIndex)>& function);

  // Process tasks meant for the main thread.
  void processMainTasks();

  int64 getWorkerCount() { return static_cast<int64>(threads.size()); }

private:

  struct Task
  {
    TaskFunction function;
    void* data;
  };

  struct TaskQueue
  {
    // Only 255 and not 256 because index 255 is reserved for invalid state.
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
