#include "wrap_map_region.h"

namespace task {

WrapMapRegion::WrapMapRegion() {};

WrapMapRegion::WrapMapRegion(boost::interprocess::mapped_region region,
                             bool has_prefix, bool has_postfix)
    : region_(std::move(region)) {
  extends_.has_prefix = has_prefix;
  extends_.has_postfix = has_postfix;
  if (extends_.has_prefix) {
    std::memcpy(extends_.prefix,
                static_cast<unsigned char *>(region_.get_address()),
                kPrefixPostfixLength);
  }
  if (extends_.has_postfix) {
    std::memcpy(extends_.postfix,
                static_cast<unsigned char *>(region_.get_address()) +
                    region_.get_size() - kPrefixPostfixLength,
                kPrefixPostfixLength);
  }

  unsigned char *valid_head =
      has_prefix ? (static_cast<unsigned char *>(region_.get_address())) +
                       kPrefixPostfixLength
                 : static_cast<unsigned char *>(region_.get_address());
  SetHeadAddress(valid_head);

  unsigned char *valid_tail =
      has_postfix ? (static_cast<unsigned char *>(region_.get_address())) +
                        region_.get_size() - kPrefixPostfixLength - 1
                  : (static_cast<unsigned char *>(region_.get_address())) +
                        region_.get_size() - 1;
  SetTailAddress(valid_tail);
}

Extends WrapMapRegion::GetExtends() { return extends_; };

void WrapMapRegion::SetHeadAddress(unsigned char *pos) { valid_head_ = pos; }
void WrapMapRegion::SetTailAddress(unsigned char *pos) { valid_tail_ = pos; }

unsigned char *WrapMapRegion::GetHeadAddress() { return valid_head_; }

unsigned char *WrapMapRegion::GetTailAddress() { return valid_tail_; }

}  // namespace task
