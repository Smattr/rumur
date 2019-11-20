#pragma once

#include <cstddef>
#include <iostream>
#include <rumur/rumur.h>

// output C code equivalent of the given node
void generate_c(const rumur::Node &n, std::ostream &out);
