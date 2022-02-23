#pragma once

#include <atomic>
#include <functional>

#include "Core/Core.hpp"

// Schedules tasks to its worker threads. Each thread is assigned its affinity to one core.
class TaskScheduler
{
public:

  using TaskFunction = void (*)(void*);

  TaskScheduler();
  TaskScheduler(const TaskScheduler& other) = delete;
  TaskScheduler(TaskScheduler&& other) = delete;
  ~TaskScheduler();

  void initialize(int threadCount, int threadAffinitiesOffset);

  void schedule(TaskFunction task, void* taskData);

  void parallelFor(int64 beginValue, int64 endValue, const std::function<void(int64)>& function);

private:

  struct Task
  {
    TaskFunction function;
    void* data;
  };

  struct TaskQueue
  {
    // Only 255 and not 256 because index 255 is reserved for invalid state. Also this way fits neatly into cacheline boundaries.
    Task tasks[255];
    std::atomic<int64> taskIndexToRead = 0;
    std::atomic<int64> taskIndexToWrite = 0;
  } queue;

  static constexpr int threadCountMax = 64;
  static constexpr uint8 invalidTaskIndex = 255;
  volatile uint8 threadCurrentTaskIndices[threadCountMax] = { invalidTaskIndex }; // fits into 1 cacheline.

  void* parallelForFinishedEvent;

  struct Thread
  {
    void* handle;
    void* stoppedEvent;
    int threadIndex;
    volatile bool shouldStop;
  };
  std::vector<Thread> threads;

private:

  static unsigned long workerThreadMain(void* parameter);

  bool taskIsBeingConsumed(int64 taskIndex) const;
};

#define DEFINE_TASK_BEGIN(taskName, TaskDataType) \
  void taskName (void* taskParameter) \
    { \
      TaskDataType& taskData = *static_cast<TaskDataType*>(taskParameter);

#define DEFINE_TASK_END \
    }