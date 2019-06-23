// some exceptions the SMT components can throw

#pragma once

#include <cstddef>
#include <stdexcept>

namespace smt {

class BudgetExhausted : public std::runtime_error {
 public:
  BudgetExhausted(): std::runtime_error("SMT solver budget exhausted") { }
};

class Unsupported : public std::runtime_error {
 public:
  Unsupported(): std::runtime_error("part of an expression was outside the "
    "currently implemented functionality") { }
};

}
