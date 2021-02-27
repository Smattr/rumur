// some exceptions the SMT components can throw

#pragma once

#include <cstddef>
#include <rumur/rumur.h>
#include <stdexcept>
#include <string>

namespace smt {

class BudgetExhausted : public std::runtime_error {
public:
  BudgetExhausted() : std::runtime_error("SMT solver budget exhausted") {}
};

class Unsupported : public std::runtime_error {
public:
  const rumur::Expr *expr = nullptr;

  explicit Unsupported()
      : std::runtime_error("part of an expression was outside the "
                           "currently implemented SMT functionality") {}

  explicit Unsupported(const rumur::Expr &expr_)
      : std::runtime_error("SMT solver encountered unsupported expression \"" +
                           expr_.to_string() + "\""),
        expr(&expr_) {}

  explicit Unsupported(const std::string &message)
      : std::runtime_error(message) {}
};

} // namespace smt
