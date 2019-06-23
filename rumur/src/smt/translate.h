#pragma once

#include <cstddef>
#include <rumur/rumur.h>
#include <string>

namespace smt {

// translate an expression to its SMTLIB equivalent
std::string translate(const rumur::Expr &expr);

}
