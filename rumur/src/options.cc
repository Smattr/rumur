#include "options.h"
#include <cstddef>
#include <string>

Options options;

// default value of "<stdin>" may be overwritten by main()
std::string input_filename = "<stdin>";
