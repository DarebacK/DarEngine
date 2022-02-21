#pragma once

#define DEFINE_TASK_BEGIN(taskName, TaskDataType) \
  void taskName (void* taskParameter) \
    { \
      TaskDataType& data = *static_cast<TaskDataType*>(taskParameter);

#define DEFINE_TASK_END \
    }