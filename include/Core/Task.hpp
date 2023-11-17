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

struct TaskThreadContext
{
  int64 index = 0; // Used in case of multiple thread per type, for example taskworker threads. Otherwise 0. Also used in parallel for, where the calling thread is 0.
};

using TaskFunction = void (*)(void* taskParameter, const TaskThreadContext& threadContext);

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

  // For events that are used just as a prerequisite.
  static Ref<TaskEvent> create();

  void complete();
  bool isComplete() const { return subsequents.isComplete; }
  void waitForCompletion() const;

private:

  // Used internally by TaskManager when scheduling a task.
  static Ref<TaskEvent> create(TaskFunction function, void* data, ThreadType desiredThread);

  TaskEvent(TaskFunction function, void* data, ThreadType desiredThread);
  TaskEvent() = default;
  TaskEvent(const TaskEvent& other) = delete;
  TaskEvent(TaskEvent&& other) = delete;
  ~TaskEvent();

  void ref();
  void unref();

  void addPrerequisite();
  void setPrerequisites(int8 prerequisites) { prerequisiteCount = prerequisites; }
  void removePrerequisite();

  bool tryAddSubsequent(Ref<TaskEvent>&& taskEvent) { return subsequents.tryAdd(std::move(taskEvent)); }
  bool tryAddSubsequent(const Ref<TaskEvent>& taskEvent) { return subsequents.tryAdd(taskEvent); }

  friend Ref<TaskEvent>;
  friend class TaskManager;

  class SubsequentList
  {
  public:

    // Returns true if the list hasn't been completed yet, false otherwise.
    bool tryAdd(Ref<TaskEvent>&& taskEvent);
    bool tryAdd(const Ref<TaskEvent>& taskEvent) { return tryAdd(Ref<TaskEvent>(taskEvent)); }

    void complete();

    struct Node
    {
      Ref<TaskEvent> taskEvent;
      Node* next = nullptr;
    };
    static FixedThreadSafePoolAllocator<Node, 2048> nodeAllocator;
    static void recycle(Node* node);

    std::atomic<Node*> head = nullptr;
    volatile bool isComplete = false;
  };
  SubsequentList subsequents;

  // It is convenient to keep this here, before the task is ready to be executed.
  TaskFunction function = nullptr;
  void* data = nullptr;
  ThreadType desiredThread = ThreadType::Unknown;

  // Initialized only when waitForCompletion() was called.
  mutable std::atomic<void*> waitableEvent = nullptr;

  std::atomic<int16> refCount = 0;
  std::atomic<int16> prerequisiteCount = 0;
};

Ref<TaskEvent> schedule(TaskFunction task, void* taskData, ThreadType desiredThread);
Ref<TaskEvent> schedule(TaskFunction task, void* taskData, ThreadType desiredThread, Ref<TaskEvent>* prerequisites, int8 prerequisiteCount);
void parallelFor(int64 beginValue, int64 endValue, const std::function<void(int64 iterationIndex, int64 threadIndex)>& function);
int64 getWorkerCount();
void processMainThreadTasks();

class TaskSystemInitializer
{
public:
  TaskSystemInitializer();
  TaskSystemInitializer(const TaskSystemInitializer& other) = delete;
  TaskSystemInitializer(TaskSystemInitializer&& other) = delete;
  ~TaskSystemInitializer();
};

#define DEFINE_TASK_BEGIN(taskName, TaskDataType) \
  void taskName (void* taskParameter, const TaskThreadContext& threadContext) \
    { \
      TRACE_SCOPE(); \
      TaskDataType& taskData = *static_cast<TaskDataType*>(taskParameter); \
      std::unique_ptr<TaskDataType> taskDataGuard{&taskData};

#define DEFINE_TASK_END \
    }
