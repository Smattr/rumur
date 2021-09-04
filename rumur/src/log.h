#pragma once

#include "options.h"
#include <cstddef>
#include <iostream>

void set_log_level(LogLevel level);

extern std::ostream *debug;
extern std::ostream *info;
extern std::ostream *warn;
