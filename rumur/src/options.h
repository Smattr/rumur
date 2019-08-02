#pragma once

#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <string>
#include <vector>

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
  mpz_class threads = 0;
  log_level_t log_level = WARNINGS;
  mpz_class set_capacity = 8 * 1024 * 1024;

  /* Limit (percentage occupancy) at which we expand the capacity of the state
   * set.
   */
  unsigned set_expand_threshold = 75;

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
  mpz_class max_errors = 1;

  // How to print counterexample traces
  counterexample_trace_t counterexample_trace = DIFF;

  // Print output as XML?
  bool machine_readable_output = false;

  // Limit for exploration. 0 means unbounded.
  mpz_class bound = 0;

  // Type used for value_t in the checker
  std::string value_type = "auto";

  // options related to SMT solver interaction
  struct {

    // Path to SMT solver. "" indicates we have no solver.
    std::string path;

    // arguments to pass to SMT solver when calling it
    std::vector<std::string> args;

    // total SMT solver execution time allowed in milliseconds
    mpz_class budget = 30000;

    // use SMT solver for expression simplification?
    bool simplification = false;

    // SMTLIB logic to use when building problems
    std::string logic = "QF_ALIA";

  } smt;
};

extern Options options;

// input model's path
extern std::string input_filename;
