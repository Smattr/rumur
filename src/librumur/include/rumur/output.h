#pragma once

#include <rumur/Model.h>
#include <string>

namespace rumur {

struct OutputOptions {
  bool overflow_checks;
  unsigned long threads;
  bool debug;
  size_t set_capacity;

  /* Limit (percentage occupancy) at which we expand the capacity of the state
   * set.
   */
  unsigned long set_expand_threshold;
};

int output_checker(const std::string &path, const Model &model,
  const OutputOptions &options);

}
