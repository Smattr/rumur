#pragma once

#include <rumur/rumur.h>

// support for comparison operations on rumur::positions

static inline bool operator<(const rumur::position &a, const rumur::position &b) {
  if (a.filename != b.filename) {
    return false;
  }
  return a.line < b.line || (a.line == b.line && a.column < b.column);
}

static inline bool operator<=(const rumur::position &a, const rumur::position &b) {
  return a < b || a == b;
}

static inline bool operator>(const rumur::position &a, const rumur::position &b) {
  return b < a;
}

static inline bool operator>=(const rumur::position &a, const rumur::position &b) {
  return a > b || a == b;
}
