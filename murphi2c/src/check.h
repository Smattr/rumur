#pragma once

#include <rumur/rumur.h>

/// validate the given AST contains no idioms that cannot be handled by murphi2c
///
/// Throws `rumur::Error` if anything is found that cannot be handled.
void check(const rumur::Node &n);
