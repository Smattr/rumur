#pragma once

#include <cstddef>
#include <rumur/rumur.h>
#include "solver.h"

namespace smt {

// emit SMT text required to define the given type
void typedecl_to_smt(Solver &solver, const rumur::TypeDecl &typedecl);

}
