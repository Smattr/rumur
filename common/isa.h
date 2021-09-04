#pragma once

#include <cstddef>

template <typename T, typename U> static inline bool isa(const U ptr) {
  return ptr != nullptr && dynamic_cast<const T *>(&*ptr) != nullptr;
}
