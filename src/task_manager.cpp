#include "task_manager.h"

namespace task {

TaskManager::TaskManager(std::filesystem::path filepath, size_t batch_size)
    : filepath_(filepath), batch_size_(batch_size) {
  file_size_ = std::filesystem::file_size(filepath);
  cur_head_pos_ = 0;
  cur_tail_pos_ = file_size_;
}

std::optional<Task> TaskManager::SetupTask() {
  std::lock_guard<std::mutex> lg(mtx_);

  if (cur_tail_pos_ <= cur_head_pos_) {
    return std::nullopt;
  }

  if (cur_tail_pos_ - cur_head_pos_ <= batch_size_) {
    Task task(std::move(boost::interprocess::file_mapping(
                  filepath_.c_str(), boost::interprocess::read_write)),
              cur_head_pos_, cur_tail_pos_ - cur_head_pos_);
    cur_head_pos_ -= (cur_tail_pos_ - cur_head_pos_);
    cur_tail_pos_ += (cur_tail_pos_ - cur_head_pos_);
    return task;
  }

  if (cur_tail_pos_ - cur_head_pos_ > batch_size_ + kMinimumBatchSize) {
    Task task(std::move(boost::interprocess::file_mapping(
                  filepath_.c_str(), boost::interprocess::read_write)),
              file_size_, cur_head_pos_, cur_tail_pos_, batch_size_);
    cur_head_pos_ -= batch_size_;
    cur_tail_pos_ += batch_size_;
    return task;
  }

  if ((cur_tail_pos_ - cur_head_pos_ > batch_size_) &&
      ((cur_tail_pos_ - cur_head_pos_) <= (batch_size_ + kMinimumBatchSize))) {
    size_t batch_size = cur_tail_pos_ - cur_head_pos_;

    Task task(std::move(boost::interprocess::file_mapping(
                  filepath_.c_str(), boost::interprocess::read_write)),
              file_size_, cur_head_pos_, cur_tail_pos_, batch_size);
    cur_head_pos_ -= batch_size;
    cur_tail_pos_ += batch_size;
    return task;
  }

  return std::nullopt;
}

}  // namespace task
