#pragma once

#include <string>

// name to give output Uclid5 module
extern std::string module_name;

// Type to use for ranges and scalarsets. Either “integer” or a bit-vector type
// “bvX”. An empty string chooses automatically between “integer” and “bv64”.
extern std::string numeric_type;

enum verbosity_t { QUIET, WARNINGS, VERBOSE };

// level of diagnostic messages to report
extern verbosity_t verbosity;
