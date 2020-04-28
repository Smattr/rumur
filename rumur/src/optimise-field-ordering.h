#pragma once

#include <cstddef>
#include <rumur/rumur.h>

// optimise the ordering of variables within the model and fields within record
// types for faster access
void optimise_field_ordering(rumur::Model &m);
