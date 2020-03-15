#pragma once

#include <cstddef>
#include <rumur/rumur.h>

// Determine whether the given AST contains any comparisons of records or
// arrays. See main.cc for why this is interesting/relevant.
bool compares_complex_values(const rumur::Node &n);
