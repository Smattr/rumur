#pragma once

#include <cstddef>
#include <string>

// escape a string for the purposes of outputting it to a C source file
std::string escape(const std::string &s);
