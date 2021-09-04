#pragma once

#include <cstddef>
#include <iostream>
#include <rumur/rumur.h>
#include <vector>

// output a C header API for the given node
///
/// \param n Node to translate
/// \param comments List of source code comments
/// \param pack Whether all structs are packed
/// \param out Stream to write translation to
void generate_h(const rumur::Node &n,
                const std::vector<rumur::Comment> &comments, bool pack,
                std::ostream &out);
