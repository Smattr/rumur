#pragma once

#include <cstddef>
#include <iostream>
#include <rumur/Model.h>
#include <rumur/Ptr.h>

namespace rumur {

// Parse in a model from an input stream. Throws Errors on parsing errors.
Ptr<Model> parse(std::istream &input);

Ptr<Model> parse(std::istream *input)
__attribute__((deprecated("parse() should be called with a reference instead of a pointer")));

}
