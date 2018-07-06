#pragma once

#include <iostream>
#include <rumur/Model.h>

namespace rumur {

/* Generate the `state_canonicalise` function for symmetry reduction. Rumur
 * generates this whether you have symmetry reduction enabled or not, but it
 * will only be used when symmetry reduction is enabled.
 */
void generate_canonicalise(const Model &m, std::ostream &out);

}
