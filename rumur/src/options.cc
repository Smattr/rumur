#include <cstddef>
#include "options.h"

Options options = {
  .overflow_checks = true,
  .threads = 0,
  .log_level = WARNINGS,
  .set_capacity = 8 * 1024 * 1024,
  .set_expand_threshold = 65,
  .color = AUTO,
  .traces = 0,
  .deadlock_detection = true,
  .symmetry_reduction = true,
  .sandbox_enabled = false,
  .max_errors = 1,
  .counterexample_trace = DIFF,
  .machine_readable_output = false,
};
