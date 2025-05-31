#include "task.h"

#include <iostream>
#include <stack>

namespace task {

Task::Task(boost::interprocess::file_mapping m_file, size_t file_size,
           size_t left_head, size_t right_tail, size_t valid_length)
    : left_region_(std::make_unique<WrapMapRegion>(WrapMapRegion(
          std::move(boost::interprocess::mapped_region(
              m_file, boost::interprocess::read_write,
              left_head == 0 ? 0 : left_head - kPrefixPostfixLength,
              left_head == 0 ? valid_length + kPrefixPostfixLength
                             : valid_length + kPrefixPostfixLength +
                                   kPrefixPostfixLength)),
          left_head == 0 ? false : true, true))),
      right_region_(std::make_unique<WrapMapRegion>(WrapMapRegion(
          std::move(boost::interprocess::mapped_region(
              m_file, boost::interprocess::read_write,
              right_tail - valid_length - kPrefixPostfixLength,
              right_tail == file_size ? valid_length + kPrefixPostfixLength
                                      : valid_length + kPrefixPostfixLength +
                                            kPrefixPostfixLength)),
          true, right_tail == file_size ? false : true))) {
  valid_length_ = valid_length;
  memory_space_ = std::make_unique<unsigned char[]>(valid_length);
}

Task::Task(boost::interprocess::file_mapping m_file, size_t left_head,
           size_t valid_length)
    : left_region_(std::make_unique<WrapMapRegion>(
          WrapMapRegion(std::move(boost::interprocess::mapped_region(
                            m_file, boost::interprocess::read_write, left_head,
                            valid_length)),
                        false, false))),
      valid_length_(valid_length),
      is_single_region_(true) {}

void Task::CopyToMemorySpace(unsigned char *src, unsigned char *dst,
                             size_t length, const Extends &extends) {
  std::memcpy(dst, src, length);
  extends_ = extends;
}

void Task::SwapAndFlip(const unsigned char *src, const unsigned char *dst,
                       size_t length, const Extends &src_extends) {
  int i = 0;
  unsigned char *head = const_cast<unsigned char *>(src);
  unsigned char *tail = const_cast<unsigned char *>(dst);

  // prefix
  if (src_extends.has_prefix) {
    if (helper::IsUTF8(src_extends.prefix[3])) {
      *tail = *head;
      tail--;
      *tail = src_extends.prefix[3];
      tail--;
      head += 2;
      i += 2;
    }
    if (helper::IsUTF8(src_extends.prefix[2])) {
      *tail = src_extends.prefix[2];
      tail--;
      head++;
      i++;
    }
  }

  // postfix
  if (src_extends.has_postfix) {
    if (helper::IsUTF8(*(head + length - 1))) {
      unsigned char *tmp = tail - length + 1;
      *tmp = src_extends.postfix[1];
      length--;
    }
    if (helper::IsUTF8(*(head + length - 2))) {
      unsigned char *tmp = tail - length + 1;
      *tmp = *(head + length - 1);
      tmp++;
      *tmp = src_extends.postfix[0];
      length -= 2;
    }
  }

  while (i < length) {
    if (helper::IsUTF8(*head)) {
      *(tail - 2) = *head;
      *(tail - 1) = *(head + 1);
      *tail = *(head + 2);

      i += 3;
      head += 3;
      tail -= 3;
      continue;
    }

    *tail = *head;

    head++;
    tail--;
    i++;
  }
}

void Task::SingleRegionFlip() {
  int i = 0, j = valid_length_ - 1;
  unsigned char *start = left_region_->GetHeadAddress();
  unsigned char *end = left_region_->GetTailAddress();

  std::stack<unsigned char> right_holder;
  std::stack<unsigned char> left_holder;

  while (i <= j - 2) {
    if (helper::IsUTF8(*start) && right_holder.size() == 0) {
      right_holder.push(*start);
      right_holder.push(*(start + 1));
      right_holder.push(*(start + 2));
    }

    if (helper::IsUTF8(*(end - 2)) && left_holder.size() == 0) {
      left_holder.push(*end);
      left_holder.push(*(end - 1));
      left_holder.push(*(end - 2));
    }

    if (right_holder.size() > 0) {
      if (left_holder.size() > 0) {
        *start = left_holder.top();
        left_holder.pop();
        *end = right_holder.top();
        right_holder.pop();

      } else {
        *start = *end;
        *end = right_holder.top();

        right_holder.pop();
      }
      start++;
      end--;
      i++;
      j--;
      continue;
    }

    if (left_holder.size() > 0) {
      if (right_holder.size() > 0) {
        *end = right_holder.top();
        right_holder.pop();
        *start = left_holder.top();
        left_holder.pop();
      } else {
        *end = *start;
        *start = left_holder.top();
        left_holder.pop();
      }
      start++;
      end--;
      i++;
      j--;
      continue;
    }

    std::swap(*start, *end);
    start++;
    end--;
    i++;
    j--;
  }

  if (left_holder.size() > 0) {
    while (left_holder.size() > 0) {
      *start = left_holder.top();
      left_holder.pop();
      start++;
    }
  } else if (right_holder.size() > 0) {
    while (right_holder.size() > 0) {
      *end = right_holder.top();
      right_holder.pop();
      end--;
    }
  }
}

void Task::Solve() {
  if (is_single_region_) {
    SingleRegionFlip();
    return;
  }

  CopyToMemorySpace(right_region_->GetHeadAddress(), &memory_space_[0],
                    valid_length_, right_region_->GetExtends());
  SwapAndFlip(left_region_->GetHeadAddress(), right_region_->GetTailAddress(),
              valid_length_, left_region_->GetExtends());
  SwapAndFlip(&memory_space_[0], left_region_->GetTailAddress(), valid_length_,
              extends_);
}

void Task::SwapAndFlipBIG5(const unsigned char *src, const unsigned char *dst,
                           size_t length, const Extends &src_extends) {
  int i = 0;
  unsigned char *head = const_cast<unsigned char *>(src);
  unsigned char *tail = const_cast<unsigned char *>(dst);

  if (src_extends.has_prefix) {
    if (!helper::IsAscii(src_extends.prefix[3])) {
      *tail = src_extends.prefix[3];
      tail--;
      head++;
      i++;
    }
  }

  if (src_extends.has_postfix) {
    if (!helper::IsAscii(*(head + length - 1))) {
      unsigned char *tmp = tail - length + 1;
      *tmp = src_extends.postfix[0];
      length--;
    }
  }

  while (i < length) {
    if (!helper::IsAscii(*head)) {
      *(tail - 1) = *head;
      *tail = *(head + 1);

      i += 2;
      head += 2;
      tail -= 2;
      continue;
    }

    *tail = *head;

    head++;
    tail--;
    i++;
  }
}

void Task::SingleRegionFlipBIG5() {
  int i = 0, j = valid_length_ - 1;
  unsigned char *start = left_region_->GetHeadAddress();
  unsigned char *end = left_region_->GetTailAddress();

  std::stack<unsigned char> right_holder;
  std::stack<unsigned char> left_holder;

  while (i <= j - 1) {
    if (!helper::IsAscii(*start) && right_holder.size() == 0) {
      right_holder.push(*start);
      right_holder.push(*(start + 1));
    }

    if (!helper::IsAscii(*(end - 1)) && left_holder.size() == 0) {
      left_holder.push(*end);
      left_holder.push(*(end - 1));
    }

    if (right_holder.size() > 0) {
      if (left_holder.size() > 0) {
        *start = left_holder.top();
        left_holder.pop();
        *end = right_holder.top();
        right_holder.pop();

      } else {
        *start = *end;
        *end = right_holder.top();

        right_holder.pop();
      }
      start++;
      end--;
      i++;
      j--;
      continue;
    }

    if (left_holder.size() > 0) {
      if (right_holder.size() > 0) {
        *end = right_holder.top();
        right_holder.pop();
        *start = left_holder.top();
        left_holder.pop();
      } else {
        *end = *start;
        *start = left_holder.top();
        left_holder.pop();
      }
      start++;
      end--;
      i++;
      j--;
      continue;
    }

    std::swap(*start, *end);
    start++;
    end--;
    i++;
    j--;
  }

  if (left_holder.size() > 0) {
    while (left_holder.size() > 0) {
      *start = left_holder.top();
      left_holder.pop();
      start++;
    }
  } else if (right_holder.size() > 0) {
    while (right_holder.size() > 0) {
      *end = right_holder.top();
      right_holder.pop();
      end--;
    }
  }
}

void Task::SolveBIG5() {
  if (is_single_region_) {
    SingleRegionFlipBIG5();
    return;
  }

  CopyToMemorySpace(right_region_->GetHeadAddress(), &memory_space_[0],
                    valid_length_, right_region_->GetExtends());
  SwapAndFlipBIG5(left_region_->GetHeadAddress(),
                  right_region_->GetTailAddress(), valid_length_,
                  left_region_->GetExtends());
  SwapAndFlipBIG5(&memory_space_[0], left_region_->GetTailAddress(),
                  valid_length_, extends_);
}
}  // namespace task