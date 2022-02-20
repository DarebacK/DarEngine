#pragma once

#include "Core/Core.hpp"

// Schedules tasks to its worker threads. Each thread is assigned its affinity to one core.
class TaskScheduler
{
public:

  using TaskFunction = void (*)(void*);

  TaskScheduler() = default;
  TaskScheduler(const TaskScheduler& other) = delete;
  TaskScheduler(TaskScheduler&& other) = delete;
  ~TaskScheduler();

  void initialize(int threadCount, int threadAffinitiesOffset);

  void schedule(TaskFunction task, void* taskData);

private:

  struct Task
  {
    TaskFunction function;
    void* data;
  };

  struct TaskQueue
  {
    Task tasks[256];
    volatile int64 taskIndexToRead = 0;
    volatile int64 taskIndexToWrite = 0;
  } queue;

};