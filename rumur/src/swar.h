/// @file
/// @brief SWAR configuration

#pragma once

#include <cstddef>
#include <rumur/rumur.h>

/// definition of a SWAR type
struct SwarShape {
  size_t lanes; ///< number of items to process in parallel
  size_t lane_width; ///< bit width of each item
};

SwarShape get_swar_type(SwarShape request, const rumur::Model &m);
