#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

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

enum deadlock_detection_t {
  DEADLOCK_DETECTION_OFF,
  DEADLOCK_DETECTION_STUCK,
  DEADLOCK_DETECTION_STUTTERING,
};

enum counterexample_trace_t {
  CEX_OFF,
  DIFF,
  FULL,
};

enum symmetry_reduction_t {
  SYMMETRY_REDUCTION_OFF,
  SYMMETRY_REDUCTION_HEURISTIC,
  SYMMETRY_REDUCTION_EXHAUSTIVE,
};

struct Options {
  unsigned long threads = 0;
  log_level_t log_level = WARNINGS;
  size_t set_capacity = 8 * 1024 * 1024;

  /* Limit (percentage occupancy) at which we expand the capacity of the state
   * set.
   */
  unsigned long set_expand_threshold = 65;

  // Whether to use ANSI colour codes in the checker's output.
  tristate color = AUTO;

  // Bitmask of enabled tracing
  uint64_t traces = 0;

  // Deadlock detection enabled?
  deadlock_detection_t deadlock_detection = DEADLOCK_DETECTION_STUTTERING;

  // Symmetry reduction enabled?
  symmetry_reduction_t symmetry_reduction = SYMMETRY_REDUCTION_HEURISTIC;

  // Use OS mechanisms to sandbox the checker?
  bool sandbox_enabled = false;

  // Number of errors to report before exiting.
  unsigned long max_errors = 1;

  // How to print counterexample traces
  counterexample_trace_t counterexample_trace = DIFF;

  // Print output as XML?
  bool machine_readable_output = false;

  // Limit for exploration. 0 means unbounded.
  unsigned long bound = 0;

  // Type used for value_t in the checker
  std::string value_type = "auto";
};

extern Options options;

// input model's path
extern std::string input_filename;
