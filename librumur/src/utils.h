#pragma once

#include <algorithm>
#include <vector>

namespace rumur {

// Compare two (possibly differently sized) vectors of pointers.
template<typename T>
bool vector_eq(const std::vector<T> &a, const std::vector<T> &b) {

  // If the two vectors are of different sizes, they cannot be equal.
  if (a.size() != b.size())
    return false;

  // Now we can just lean on the standard library
  return std::equal(a.begin(), a.end(), b.begin(),
    [](const T x, const T y) {
      return *x == *y;
    });
}

template<typename T, typename U>
bool isa(const U ptr) {
  return ptr != nullptr && dynamic_cast<const T*>(&*ptr) != nullptr;
}

}
