#pragma once

#include <cstddef>
#include <gmpxx.h>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace smt {

class Solver {

private:
  std::vector<std::shared_ptr<std::ostringstream>> prelude;
  mpz_class time_used = 0;

  enum Result { SAT, UNSAT, INCONCLUSIVE };

  /* Using the accrued prelude setup declarations, try to prove that the claim
   * expression is the expectation. I.e. prove the claim true or false depending
   * on whether expectation is true or false. For those unfamiliar with SMT
   * solvers, the way to interpret the result is:
   *  SAT - there is a value(s) for which claim != expectation (proof failed)
   *  UNSAT - for all values claim == expectation (proof succeeded)
   *  INCONCLUSIVE - resource exhaustion, e.g. timeout
   */
  Result solve(const std::string &claim, bool expectation);

public:
  // can this expression be proven always-true?
  bool is_true(const std::string &claim);

  // can this expression be proven always-false?
  bool is_false(const std::string &claim);

  // add something to the prelude (e.g. a declaration "(declare-fun v () Int)")
  Solver &operator<<(const std::string &s);

  // open a new (nested) variable scope
  void open_scope();

  // close a scope
  void close_scope();
};

} // namespace smt
