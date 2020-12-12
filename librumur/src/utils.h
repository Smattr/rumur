#pragma once

#include <cstddef>

namespace rumur {

template<typename T, typename U>
bool isa(const U ptr) {
  return ptr != nullptr && dynamic_cast<const T*>(&*ptr) != nullptr;
}

}
