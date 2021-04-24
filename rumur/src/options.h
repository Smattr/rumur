#pragma once

#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <string>
#include <vector>

enum struct Color {
  OFF,
  ON,
  AUTO,
};

enum trace_category_t {
  TC_HANDLE_READS       =  0x1,
  TC_HANDLE_WRITES      =  0x2,
  TC_MEMORY_USAGE       =  0x4,
  TC_QUEUE              =  0x8,
  TC_SET                = 0x10,
  TC_SYMMETRY_REDUCTION = 0x20,
};

enum struct LogLevel {
  SILENT,
  WARNINGS,
  INFO,
  DEBUG,
};

enum struct DeadlockDetection {
  OFF,
  STUCK,
  STUTTERING,
};

enum struct CounterexampleTrace {
  OFF,
  DIFF,
  FULL,
};

enum struct SymmetryReduction {
  OFF,
  HEURISTIC,
  EXHAUSTIVE,
};

enum struct SmtSimplification {
  OFF,
  ON,
  AUTO,
};

struct Options {
  mpz_class threads = 0;
  LogLevel log_level = LogLevel::WARNINGS;
  mpz_class set_capacity = 8 * 1024 * 1024;

  /* Limit (percentage occupancy) at which we expand the capacity of the state
   * set.
   */
  unsigned set_expand_threshold = 75;

  // Whether to use ANSI colour codes in the checker's output.
  Color color = Color::AUTO;

  // Bitmask of enabled tracing
  uint64_t traces = 0;

  // Deadlock detection enabled?
  DeadlockDetection deadlock_detection = DeadlockDetection::STUTTERING;

  // Symmetry reduction enabled?
  SymmetryReduction symmetry_reduction = SymmetryReduction::HEURISTIC;

  // Use OS mechanisms to sandbox the checker?
  bool sandbox_enabled = false;

  // Number of errors to report before exiting.
  mpz_class max_errors = 1;

  // How to print counterexample traces
  CounterexampleTrace counterexample_trace = CounterexampleTrace::DIFF;

  // Print output as XML?
  bool machine_readable_output = false;

  // Limit for exploration. 0 means unbounded.
  mpz_class bound = 0;

  // Type used for value_t in the checker
  std::string value_type = "auto";

  // whether to bit-pack members of the state struct
  bool pack_state = true;

  // whether to optimise state variable and record fields ordering
  bool reorder_fields = true;

  // whether to track schedules during scalarset permutation
  bool scalarset_schedules = true;

  // number of relevant bits in a pointer on the target platform (0 == auto)
  mpz_class pointer_bits = 0;

  // options related to SMT solver interaction
  struct {

    // Path to SMT solver. "" indicates we have no solver.
    std::string path;

    // arguments to pass to SMT solver when calling it
    std::vector<std::string> args;

    // total SMT solver execution time allowed in milliseconds
    mpz_class budget = 30000;

    // use SMT solver for expression simplification?
    SmtSimplification simplification = SmtSimplification::AUTO;

    // text to emit to the solver prior to a SAT problem
    std::vector<std::string> prelude;

    // use BitVecs instead of Ints?
    bool use_bitvectors = false;

  } smt;
};

extern Options options;

// input model's path
extern std::string input_filename;
