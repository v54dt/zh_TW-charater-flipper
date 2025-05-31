#ifndef TASK_MANAGER_H_
#define TASK_MANAGER_H_

#include <boost/interprocess/file_mapping.hpp>
#include <filesystem>
#include <optional>
#include <queue>
#include <string>

#include "task.h"

namespace task {

class TaskManager {
 public:
  // copyable
  TaskManager(const TaskManager& rhs) = default;
  TaskManager& operator=(const TaskManager& rhs) = default;
  // movable
  TaskManager(TaskManager&& rhs) = default;
  TaskManager& operator=(TaskManager&& rhs) = default;

  TaskManager(std::filesystem::path filepath, size_t batch_size);

  std::optional<Task> SetupTask();

 private:
  std::filesystem::path filepath_;
  size_t file_size_;
  size_t batch_size_;
  const size_t kMinimumBatchSize = 8;
  size_t cur_head_pos_;
  size_t cur_tail_pos_;
  bool is_finished_;

 private:
  std::mutex mtx_;
  std::queue<Task> task_queue_;
};

}  // namespace task

#endif
