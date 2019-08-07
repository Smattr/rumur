#pragma once

#include <cstddef>
#include <rumur/rumur.h>
#include "solver.h"

namespace smt {

/* declare any enum members that are syntactically contained under the given
 * type
 */
void define_enum_members(Solver &solver, const rumur::TypeExpr &type);

}
