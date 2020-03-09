#pragma once

#include <cstddef>
#include <iostream>
#include <rumur/rumur.h>

// Output C code equivalent of the given node. The `pack` parameter determines
// whether all structs are packed.
void generate_c(const rumur::Node &n, bool pack, std::ostream &out);
