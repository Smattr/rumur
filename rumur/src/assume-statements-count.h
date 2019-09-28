#pragma once

#include <cstddef>
#include <rumur/rumur.h>

// find the number of assume statements in the model
unsigned long assume_statements_count(const rumur::Model &model);
