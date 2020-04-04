#pragma once

#include <cstddef>

// global options set by command line flags
struct Options {

  // add Rumur-optional semicolons for compatibility with CMurphi?
  bool explicit_semicolons = false;

  // remove liveness invariants and statements?
  bool remove_liveness = false;

  // turn switch statements into if statements?
  bool switch_to_if = false;

  // turn unicode operators into their ASCII equivalents?
  bool to_ascii = false;

  // turn complex ==/!= into simple ==/!=s?
  bool decompose_complex_comparisons = false;
};

extern Options options;
