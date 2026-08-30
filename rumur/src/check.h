#pragma once

#include <rumur/rumur.h>

/// throw a `rumur::Error` if the given node contains anything unsupported
void check(const rumur::Node &n);
