#pragma once

#include <rumur/rumur.h>

// Support for comparison on rumur::positions. It is reasonable to wonder why we
// do not just implement the C++ comparison operators on rumur::position or why
// Bison itself does not implement these. It is not possible to define a total
// order on positions because two positions may have come from different files
// and thus be incomparable. Implementing the C++ operators using a notion of
// ordering that is not strict and total is dangerous. For more information:
//  - https://lists.gnu.org/archive/html/help-bison/2019-11/msg00000.html
//  - https://foonathan.net/2018/07/ordering-relations-programming/

static inline bool is_leq(const rumur::position &a, const rumur::position &b) {
  return a.line < b.line || (a.line == b.line && a.column < b.column);
}
