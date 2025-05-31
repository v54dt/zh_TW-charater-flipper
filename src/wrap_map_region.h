#ifndef WRAP_MAP_REGION_H_
#define WRAP_MAP_REGION_H_

#include <boost/interprocess/mapped_region.hpp>

namespace task {

struct Extends {
  char prefix[4];
  char postfix[4];
  bool has_prefix;
  bool has_postfix;
};

class WrapMapRegion {
 public:
  // copyable
  WrapMapRegion(const WrapMapRegion& rhs) = default;
  WrapMapRegion& operator=(const WrapMapRegion& rhs) = default;
  // movable
  WrapMapRegion(WrapMapRegion&& rhs) = default;
  WrapMapRegion& operator=(WrapMapRegion&& rhs) = default;

 public:
  WrapMapRegion();

  WrapMapRegion(boost::interprocess::mapped_region region, bool has_prefix,
                bool has_postfix);

 public:
  unsigned char* GetHeadAddress();
  unsigned char* GetTailAddress();
  void SetHeadAddress(unsigned char* pos);
  void SetTailAddress(unsigned char* pos);
  Extends GetExtends();

 private:
  boost::interprocess::mapped_region region_;
  const size_t kPrefixPostfixLength = 4;
  unsigned char* valid_head_;
  unsigned char* valid_tail_;
  Extends extends_;
};

}  // namespace task

#endif
