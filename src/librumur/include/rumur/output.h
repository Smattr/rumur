#pragma once

#include <rumur/Model.h>
#include <string>

namespace rumur {

enum tristate {
  OFF,
  ON,
  AUTO,
};

enum trace_category_t {
  TC_HANDLE_READS  = 0x1,
  TC_HANDLE_WRITES = 0x2,
  TC_QUEUE         = 0x4,
  TC_SET           = 0x8,
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

  // Bitmask of enabled tracing
  uint64_t traces;

  // Deadlock detection enabled?
  bool deadlock_detection;

  // Symmetry reduction enabled?
  bool symmetry_reduction;
};

int output_checker(const std::string &path, const Model &model,
  const OutputOptions &options);

}
