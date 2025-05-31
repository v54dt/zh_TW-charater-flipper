#include <boost/atomic.hpp>
#include <condition_variable>
#include <filesystem>
#include <thread>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "task_manager.h"

ABSL_FLAG(int, parallels, 16, "grade of parallels");
ABSL_FLAG(std::string, filepath, "../test_file.txt", "path of imput file");
ABSL_FLAG(size_t, batch_size, 4088, "how many byte to swap of one task");
ABSL_FLAG(std::string, type, "utf8", "UTF8 or BIG5");

std::condition_variable queue_ready;
std::atomic_bool produce_finished(false);
std::atomic_bool task_finished(false);
std::queue<task::Task> queue;
std::atomic_int queue_size(0);
std::condition_variable queue_not_full;

std::mutex consumer_mtx;
std::mutex producer_mtx;

static int parallels;

void Producer(task::TaskManager& task_manager) {
  while (1) {
    auto task = task_manager.SetupTask();
    if (!task.has_value()) {
      produce_finished = true;
      queue_ready.notify_all();
      break;
    }
    while (queue_size == parallels) {
      std::unique_lock<std::mutex> lck(producer_mtx);
      queue_not_full.wait(lck);
    }

    queue.push(std::move(task.value()));
    queue_size++;
    if (queue.size() >= 2) {
      queue_ready.notify_all();
    }
  }
}

task::Task ConsumerGetTask() {
  auto task = std::move(queue.front());
  queue.pop();
  return std::move(task);
}

void Consumer(std::string type) {
  while (1) {
    std::unique_lock<std::mutex> lck(consumer_mtx);
    if (!task_finished) {
      queue_ready.wait(lck);  // block consumer here when starts
    } else {
      queue_ready.notify_all();  // notify other consumer to move on
      break;
    }
    if (produce_finished) {
      if (queue_size <= 0) {
        task_finished = true;  // producer finished and queue is empty means
                               // whole tasks is done
        lck.unlock();
        queue_ready.notify_all();
        break;
      } else {
        auto task = ConsumerGetTask();  // all task has push to queue so don`t
                                        // need to check queue size
        queue_size--;
        if (queue_size == 0) task_finished = true;
        lck.unlock();
        task.Solve();
        continue;
      }
    }
    // ensure queue contains at least two tasks to prevent the second task`s
    // prefix from being overwritten by first task
    if (queue_size < 2) {
      lck.unlock();
      continue;
    }

    auto task = ConsumerGetTask();
    queue_size--;
    queue_not_full.notify_all();
    lck.unlock();

    if (type == "utf8") {
      task.Solve();
    }
    if (type == "big5") {
      task.SolveBIG5();
    }
  }
}

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  parallels = absl::GetFlag(FLAGS_parallels);
  std::string large_file = absl::GetFlag(FLAGS_filepath);
  size_t batch_size = absl::GetFlag(FLAGS_batch_size);
  std::string type = absl::GetFlag(FLAGS_type);

  std::filesystem::path filepath = large_file;
  size_t file_size = std::filesystem::file_size(filepath);

  task::TaskManager task_manager(filepath, batch_size);

  std::thread producer(Producer, std::ref(task_manager));

  std::vector<std::thread> consumers;
  for (int i = 0; i < parallels; i++) {
    consumers.emplace_back(std::thread(Consumer, type));
  }

  producer.join();
  for (auto& consumer : consumers) consumer.join();

  return 0;
}
