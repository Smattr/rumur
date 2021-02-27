#pragma once

#include <cstddef>
#include <gmpxx.h>
#include <iostream>
#include <rumur/rumur.h>
#include <vector>

// find all the named scalarset declarations in a model
std::vector<const rumur::TypeDecl *> get_scalarsets(const rumur::Model &m);

// get the number of bits required to store a permutation index for the given
// scalarset
mpz_class get_schedule_width(const rumur::TypeDecl &t);

/* Generate the `state_canonicalise` function for symmetry reduction. Rumur
 * generates this whether you have symmetry reduction enabled or not, but it
 * will only be used when symmetry reduction is enabled.
 */
void generate_canonicalise(const rumur::Model &m, std::ostream &out);
