#pragma once

#include <cstddef>
#include <rumur/rumur.h>
#include <string>

namespace smt {

// translate an expression to its SMTLIB equivalent
std::string translate(const rumur::Expr &expr);

// name-mangle a symbol to make it a safe SMT variable
std::string mangle(const std::string &s, size_t id);

} // namespace smt
