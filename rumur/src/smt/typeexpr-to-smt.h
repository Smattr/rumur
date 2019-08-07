#pragma once

#include <cstddef>
#include <string>
#include <rumur/rumur.h>

namespace smt {

// translate a Rumur type into the equivalent SMTLIB type expression
std::string typeexpr_to_smt(const rumur::TypeExpr &type);

}
