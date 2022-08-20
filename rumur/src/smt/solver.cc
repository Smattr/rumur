#include "solver.h"
#include "../log.h"
#include "../options.h"
#include "../process.h"
#include "except.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <gmpxx.h>
#include <memory>
#include <sstream>
#include <string>

namespace smt {

// some helper functions to contain the visually noisy chrono API

typedef std::chrono::time_point<std::chrono::system_clock> timestamp_t;

static timestamp_t get_timestamp() { return std::chrono::system_clock::now(); }

static mpz_class get_duration(const timestamp_t &start,
                              const timestamp_t &end) {
  std::chrono::duration<double> duration = end - start;
  return duration.count() * 1000;
}

Solver::Result Solver::solve(const std::string &claim, bool expectation) {

  if (time_used >= options.smt.budget)
    throw BudgetExhausted();

  std::ostringstream query;

  // disable printing of "success" in response to commands
  query << "(set-option :print-success false)\n";

  // write any prelude the user requested
  for (const std::string &text : options.smt.prelude) {
    query << text << "\n";
  }

  // append the declarations etc
  for (const std::shared_ptr<std::ostringstream> &scope : prelude)
    query << scope->str();

  // set up the main claim
  query << "(assert " << (expectation ? "(not " : "") << claim
        << (expectation ? ")" : "") << ")\n"
        << "(check-sat)\n";

  *debug << "checking SMT problem:\n" << query.str();

  // construct the call to the solver
  std::vector<std::string> args;
  assert(options.smt.path != "" &&
         "calling SMT solver without having supplied a path to it");
  args.push_back(options.smt.path);
  std::copy(options.smt.args.begin(), options.smt.args.end(),
            std::back_inserter(args));

  auto start = get_timestamp();

  std::string output;
  int r = run(args, query.str(), output);

  auto end = get_timestamp();

  time_used += get_duration(start, end);

  if (r < 0) {
    *debug << "SMT solver error\n";
    return INCONCLUSIVE;
  }

  *debug << "SMT solver said:\n" << output;

  // look for a "sat" or "unsat" line
  std::istringstream ss(output);
  for (std::string line; std::getline(ss, line);) {
    if (line == "sat")
      return SAT;
    if (line == "unsat")
      return UNSAT;
  }

  *debug << "inconclusive result from SMT solver\n";

  return INCONCLUSIVE;
}

bool Solver::is_true(const std::string &claim) {
  return solve(claim, true) == UNSAT;
}

bool Solver::is_false(const std::string &claim) {
  return solve(claim, false) == UNSAT;
}

Solver &Solver::operator<<(const std::string &s) {
  assert(!prelude.empty() && "writing SMT content without an open scope");
  *prelude[prelude.size() - 1] << s;
  return *this;
}

/* you might think we could just use "(push)" and "(pop)" for scoping, but some
 * SMT solvers only support these in incremental mode and we're calling the
 * solver in one-shot mode
 */

void Solver::open_scope() {
  prelude.push_back(std::make_shared<std::ostringstream>());
}

void Solver::close_scope() {
  assert(!prelude.empty() && "closing a scope when none are open");
  prelude.pop_back();
}

} // namespace smt
