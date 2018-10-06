#pragma once

#include <cstddef>
#include <cstdint>

enum tristate {
  OFF,
  ON,
  AUTO,
};

enum trace_category_t {
  TC_HANDLE_READS       =  0x1,
  TC_HANDLE_WRITES      =  0x2,
  TC_QUEUE              =  0x4,
  TC_SET                =  0x8,
  TC_SYMMETRY_REDUCTION = 0x10,
};

enum log_level_t {
  SILENT,
  WARNINGS,
  INFO,
  DEBUG,
};

enum counterexample_trace_t {
  CEX_OFF,
  DIFF,
  FULL,
};

struct Options {
  bool overflow_checks;
  unsigned long threads;
  log_level_t log_level;
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

  // Use OS mechanisms to sandbox the checker?
  bool sandbox_enabled;

  // Number of errors to report before exiting.
  unsigned long max_errors;

  // How to print counterexample traces
  counterexample_trace_t counterexample_trace;

  // Print output as XML?
  bool machine_readable_output;
};

extern Options options;
