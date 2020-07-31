#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

namespace rumur {

template<typename T, typename U>
bool isa(const U ptr) {
  return ptr != nullptr && dynamic_cast<const T*>(&*ptr) != nullptr;
}

}
