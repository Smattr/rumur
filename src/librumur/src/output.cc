#include <iostream>
#include <fstream>
#include <rumur/Model.h>
#include <rumur/output.h>
#include "resources.h"
#include <string>
#include <utility>
#include <vector>

namespace rumur {

/* Turn a string into a something that can be safely emitted in a C++ file and
 * seen by the compiler as a string literal.
 */
static std::string escape_string(const std::string &s) {
    // TODO
    return "\"" + s + "\"";
}

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
      << "using ModelError = ModelErrorBase<State>;\n";

    // Write out constants and type declarations.
    for (const Decl *d : model.decls)
        d->define(out);

    // Write out the start state rules.
    {
        std::vector<std::string> start_rules;
        for (const Rule *r : model.rules) {
            if (auto s = dynamic_cast<const StartState*>(r)) {
                out << "void startstate_" << start_rules.size() << "(State &s) {\n";
                s->generate_rule(out);
                out << "}\n\n";
                start_rules.push_back(s->name);
            }
        }

        out << "static const std::vector<StartState> START_RULES = {\n";
        unsigned i = 0;
        for (const std::string &s : start_rules) {
            out << "    { .name = " << escape_string(s) << ", .body = startstate_" << i << "},\n";
            i++;
        }
        out << "};\n\n";
    }

    // Write out the invariant rules.
    {
        std::vector<std::string> invariants;
        for (const Rule *r : model.rules) {
            if (auto i = dynamic_cast<const Invariant*>(r)) {
                out << "static bool invariant_" << invariants.size() << "(const State &s) {\n"
                       "    // TODO\n"
                       "    return true;\n"
                       "}\n\n";
                invariants.push_back(i->name);
            }
        }

        out << "static const std::vector<Invariant> INVARIANTS = {\n";
        unsigned i = 0;
        for (const std::string &n : invariants) {
            out << "    { .name = " << escape_string(n) << ", .guard = invariant_" << i << "},\n";
            i++;
        }
        out << "};\n\n";
    }

    // Write out the regular rules.
    {
        std::vector<std::string> rules;
        for (const Rule *r : model.rules) {
            if (is_regular_rule(r)) {
                out << "static bool guard_" << rules.size() << "(const State &s) {\n"
                       "    //TODO\n"
                       "    return true;\n"
                       "}\n\n"
                       "static void rule_" << rules.size() << "(State &s) {\n"
                       "    // TODO\n"
                       "}\n\n";
                rules.push_back(r->name);
            }
        }

        out << "static const std::vector<Rule> RULES = {\n";
        unsigned i = 0;
        for (const std::string &n : rules) {
            out << "    { .name = " << escape_string(n) << ", .guard = guard_" <<
              i << ", .body = rule_" << i << "},\n";
            i++;
        }
        out << "};\n\n";
    }

    out << std::string((const char*)resources_footer_cc, (size_t)resources_footer_cc_len);

    return 0;

}

}
