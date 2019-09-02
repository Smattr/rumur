#pragma once

#include <cstddef>
#include <rumur/rumur.h>
#include "solver.h"

namespace smt {

void define_records(Solver &solver, const rumur::TypeExpr &type);

}
