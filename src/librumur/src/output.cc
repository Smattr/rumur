#include <iostream>
#include <fstream>
#include <rumur/Model.h>
#include <rumur/output.h>
#include "resources.h"
#include <string>
#include <utility>
#include <vector>

namespace rumur {

// Whether a rule is a standard state transition rule.
static bool is_regular_rule(const Rule *r) {
  return dynamic_cast<const StartState*>(r) == nullptr &&
         dynamic_cast<const Invariant*>(r) == nullptr;
}

int output_checker(const std::string &path, const Model &model,
  const OutputOptions &options) {

  std::ofstream out(path);
  if (!out)
    return -1;

  out

    // Settings that are used in header.cc
    << "static constexpr bool OVERFLOW_CHECKS_ENABLED = " <<
    (options.overflow_checks ? "true" : "false") << ";\n"

    // Static boiler plate code
    << std::string((const char*)resources_header_cc, (size_t)resources_header_cc_len)
    << "\n"

    // Specialise classes
    << "using State = StateBase<" << model.size_bits() << ">;\n"
    << "using StartState = StartStateBase<State>;\n"
    << "using Invariant = InvariantBase<State>;\n"
    << "using Rule = RuleBase<State>;\n"
    << "using ModelError = ModelErrorBase<State>;\n\n";

  // Write out constants and type declarations.
  for (const Decl *d : model.decls)
    out << *d << ";\n";
  out << "\n";

  // Write out the start state rules.
  out << "static const std::vector<StartState> START_RULES = {\n";
  for (const Rule *r : model.rules) {
    if (auto s = dynamic_cast<const StartState*>(r))
      out << *s << ",\n";
  }
  out << "};\n\n";

  // Write out the invariant rules.
  out << "static const std::vector<Invariant> INVARIANTS = {\n";
  for (const Rule *r : model.rules) {
    if (auto i = dynamic_cast<const Invariant*>(r))
      out << *i << ";\n";
  }
  out << "};\n\n";

  // Write out the regular rules.
  out << "static const std::vector<Rule> RULES = {\n";
  for (const Rule *r : model.rules) {
    if (is_regular_rule(r))
      out << *r << ";\n";
  }
  out << "};\n\n";

  out << std::string((const char*)resources_footer_cc, (size_t)resources_footer_cc_len);

  return 0;

}

}
