#pragma once

#include <rumur/rumur.h>
#include <vector>

// find the source locations of all liveness properties in the model
std::vector<rumur::location> find_liveness(const rumur::Model &model);
