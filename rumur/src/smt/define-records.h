#pragma once

#include "solver.h"
#include <cstddef>
#include <rumur/rumur.h>

namespace smt {

void define_records(Solver &solver, const rumur::TypeExpr &type);

}
