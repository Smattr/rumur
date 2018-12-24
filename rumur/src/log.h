#pragma once

#include <cstddef>
#include <iostream>
#include "options.h"

void set_log_level(log_level_t level);

extern std::ostream *debug;
extern std::ostream *info;
extern std::ostream *warn;
