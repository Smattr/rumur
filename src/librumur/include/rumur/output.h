#pragma once

#include <rumur/Model.h>
#include <string>

namespace rumur {

enum tristate {
  OFF,
  ON,
  AUTO,
};

struct OutputOptions {
  bool overflow_checks;
  unsigned long threads;
  bool debug;
  size_t set_capacity;

  /* Limit (percentage occupancy) at which we expand the capacity of the state
   * set.
   */
  unsigned long set_expand_threshold;

  // Whether to use ANSI colour codes in the checker's output.
  tristate color;
};

int output_checker(const std::string &path, const Model &model,
  const OutputOptions &options);

}
