#pragma once

#include <cstddef>
#include <rumur/rumur.h>
#include <string>

namespace smt {

// translate a Rumur type into the equivalent SMTLIB type expression
std::string typeexpr_to_smt(const rumur::TypeExpr &type);

} // namespace smt
