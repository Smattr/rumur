#pragma once

#include <string>

// name to give output Uclid5 module
extern std::string module_name;

enum verbosity_t { QUIET, WARNINGS, VERBOSE };

// level of diagnostic messages to report
extern verbosity_t verbosity;
