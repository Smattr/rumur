#pragma once

#include <rumur/rumur.h>

// validate the given AST contains no idioms that cannot be handled by murphi2c,
// and return false if any are found
bool check(const rumur::Node &n);
