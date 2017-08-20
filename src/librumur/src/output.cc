#include <iostream>
#include <fstream>
#include <memory>
#include <rumur/Model.h>
#include <rumur/output.h>
#include "resources.h"
#include <string>
#include <vector>

using namespace rumur;
using namespace std;

/* Turn a string into a something that can be safely emitted in a C++ file and
 * seen by the compiler as a string literal.
 */
static string escape_string(const string &s) {
    // TODO
    return "\"" + s + "\"";
}

// Whether a rule is a standard state transition rule.
static bool is_regular_rule(const shared_ptr<Rule> r) {
    return dynamic_pointer_cast<const StartState>(r) == nullptr &&
           dynamic_pointer_cast<const Invariant>(r) == nullptr;
}

int rumur::output_checker(const string &path, const Model &model,
  const OutputOptions &options) {

    ofstream out(path);
    if (!out)
        return -1;

#define WRITE(resource) \
    do { \
        for (unsigned int i = 0; i < resources_##resource##_len; i++) { \
            out << resources_##resource[i]; \
        } \
        out << "\n"; \
    } while (0)

    WRITE(includes_cc);

    out
      << "static constexpr bool OVERFLOW_CHECKS_ENABLED = " <<
      (options.overflow_checks ? "true" : "false") << ";\n"

      << "static constexpr uint64_t STATE_SIZE_BITS = " << model.size_bits()
      << ";\n"

      << "\n";

    WRITE(State_cc);

    WRITE(header_cc);

    // Write out the start state rules.
    {
        vector<string> start_rules;
        for (const shared_ptr<Rule> r : model.rules) {
            if (auto s = dynamic_pointer_cast<const StartState>(r)) {
                out << "static State *startstate_" << start_rules.size() << "() {\n"
                       "    State *s = new State;\n"
                       "    // TODO\n"
                       "    return s;\n"
                       "}\n\n";
                start_rules.push_back(s->name);
            }
        }

        out << "static const std::array<std::pair<std::string, std::function<State*()>>, "
          << start_rules.size() << "> START_RULES = {\n";
        unsigned i = 0;
        for (const string &s : start_rules) {
            out << "    std::make_pair(" << escape_string(s) << ", startstate_" << i << "),\n";
            i++;
        }
        out << "};\n\n";
    }

    // Write out the invariant rules.
    {
        vector<string> invariants;
        for (const shared_ptr<Rule> r : model.rules) {
            if (auto i = dynamic_pointer_cast<const Invariant>(r)) {
                out << "static bool invariant_" << invariants.size() << "(const State &s [[maybe_unused]]) {\n"
                       "    // TODO\n"
                       "    return true;\n"
                       "}\n\n";
                invariants.push_back(i->name);
            }
        }

        out << "static const std::array<std::pair<std::string, "
            "std::function<bool(const State&)>>, " << invariants.size() <<
            "> INVARIANTS = {\n";
        unsigned i = 0;
        for (const string &n : invariants) {
            out << "    std::make_pair(" << escape_string(n) << ", invariant_" << i << "),\n";
            i++;
        }
        out << "};\n\n";
    }

    // Write out the regular rules.
    {
        vector<string> rules;
        for (const shared_ptr<Rule> r : model.rules) {
            if (is_regular_rule(r)) {
                out << "static bool guard_" << rules.size() << "(const State &s [[maybe_unused]]) {\n"
                       "    //TODO\n"
                       "    return true;\n"
                       "}\n\n"
                       "static void rule_" << rules.size() << "(State *s [[maybe_unused]]) {\n"
                       "    // TODO\n"
                       "}\n\n";
                rules.push_back(r->name);
            }
        }

        out << "static const std::array<std::tuple<std::string, "
          "std::function<bool(const State&)>, std::function<void(State*)>>, " <<
          rules.size() << "> RULES = {\n";
        unsigned i = 0;
        for (const string &n : rules) {
            out << "    std::make_tuple(" << escape_string(n) << ", guard_" <<
              i << ", rule_" << i << "),\n";
            i++;
        }
        out << "};\n\n";
    }

    WRITE(main_cc);

#undef WRITE

    return 0;

}
