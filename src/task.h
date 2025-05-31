#ifndef TASK_H_
#define TASK_H_

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <memory>
#include <vector>

#include "helper.h"
#include "wrap_map_region.h"

namespace task {

class Task {
 public:
  // copyable
  Task(const Task& rhs) = default;
  Task& operator=(const Task& rhs) = default;
  // movable
  Task(Task&& rhs) = default;
  Task& operator=(Task&& rhs) = default;

 public:
  Task(boost::interprocess::file_mapping m_file, size_t file_size,
       size_t left_head, size_t right_tail, size_t valid_length);
  Task(boost::interprocess::file_mapping m_file, size_t left_head,
       size_t valid_length);

 public:
  void Solve();
  void CopyToMemorySpace(unsigned char* src, unsigned char* dst, size_t length,
                         const Extends& extends);
  void SwapAndFlip(const unsigned char* src, const unsigned char* dst,
                   size_t length, const Extends& extends);
  void SingleRegionFlip();

  void SolveBIG5();
  void SwapAndFlipBIG5(const unsigned char* src, const unsigned char* dst,
                       size_t length, const Extends& extends);
  void SingleRegionFlipBIG5();

 private:
  const size_t kPrefixPostfixLength = 4;
  size_t file_size_;
  size_t valid_length_;
  bool is_single_region_ = false;
  std::__detail::__unique_ptr_array_t<unsigned char[]> memory_space_;
  Extends extends_;

 private:
  std::shared_ptr<WrapMapRegion> left_region_;
  std::shared_ptr<WrapMapRegion> right_region_;
};

}  // namespace task

#endif
