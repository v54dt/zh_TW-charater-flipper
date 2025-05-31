#include "helper.h"

#include <stack>

namespace helper {

bool IsAscii(const unsigned char byte) { return byte >= 0x20 && byte <= 0x7f; }
bool IsUTF8(const unsigned char byte) { return byte >= 0xe0; }

}  // namespace helper
