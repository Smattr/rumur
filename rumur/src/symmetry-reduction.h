#pragma once

#include <cstddef>
#include <iostream>
#include <rumur/rumur.h>

/* Generate the `state_canonicalise` function for symmetry reduction. Rumur
 * generates this whether you have symmetry reduction enabled or not, but it
 * will only be used when symmetry reduction is enabled.
 */
void generate_canonicalise(const rumur::Model &m, std::ostream &out);
