#pragma once

#include "solver.h"
#include <cstddef>
#include <rumur/rumur.h>

namespace smt {

/* declare any enum members that are syntactically contained under the given
 * type
 */
void define_enum_members(Solver &solver, const rumur::TypeExpr &type);

} // namespace smt
