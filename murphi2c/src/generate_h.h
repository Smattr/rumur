#pragma once

#include <cstddef>
#include <iostream>
#include <rumur/rumur.h>

// Output a C header API for the given node. The `pack` parameter determines
// whether all structs are packed.
void generate_h(const rumur::Node &n, bool pack, std::ostream &out);
