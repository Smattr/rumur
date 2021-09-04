#pragma once

#include <cstddef>
#include <iostream>
#include <rumur/Model.h>
#include <rumur/Ptr.h>

#ifndef RUMUR_API
#define RUMUR_API __attribute__((visibility("default")))
#endif

namespace rumur {

// Parse in a model from an input stream. Throws Errors on parsing errors.
RUMUR_API Ptr<Model> parse(std::istream &input);

} // namespace rumur
