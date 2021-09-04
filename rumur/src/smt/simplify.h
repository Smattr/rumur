#pragma once

#include <rumur/rumur.h>

namespace smt {

/* Attempt to simplify expressions within a model using an external
 * Satisfiability Modulo Theories (SMT) solver.
 */
void simplify(rumur::Model &model);

} // namespace smt
