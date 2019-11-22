#pragma once

#include <rumur/rumur.h>

// validate the given AST contains no idioms that cannot be handled by murphi2c,
// and exit if any are found
void check(const rumur::Node &n);
