#pragma once

#include <cstddef>
#include <rumur/rumur.h>
#include <string>

/// pick a numeric type based on the content of a model
///
/// \param n Model to examine
/// \return Automatically selected numeric type
std::string pick_numeric_type(const rumur::Node &n);
