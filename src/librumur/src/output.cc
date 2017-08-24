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

    out << "#include <stdbool.h>\n"
           "#include <stdint.h>\n"
           "#include <stdio.h>\n"
           "#include <unistd.h>\n";

#define WRITE(resource) \
    do { \
        for (unsigned int i = 0; i < resources_##resource##_len; i++) { \
            out << resources_##resource[i]; \
        } \
        out << "\n"; \
    } while (0)

    out
      << "enum { OVERFLOW_CHECKS_ENABLED = " <<
      (options.overflow_checks ? "true" : "false") << "};\n"

      << "enum { STATE_SIZE_BITS = " << model.size_bits()
      << "};\n"

      << "\n";

    WRITE(State_c);

    WRITE(collections_c);

    WRITE(header_c);

    // Write out the start state rules.
    {
        vector<string> start_rules;
        for (const shared_ptr<Rule> r : model.rules) {
            if (auto s = dynamic_pointer_cast<const StartState>(r)) {
                out << "static State *startstate_" << start_rules.size() << "() {\n"
                       "    State *s = malloc(sizeof(*s));\n"
                       "    if (s == NULL) {\n"
                       "        perror(\"malloc\");\n"
                       "        exit(EXIT_FAILURE);\n"
                       "    }\n";
                s->write_rule(out, "    ");
                out << "    return s;\n"
                       "}\n\n";
                start_rules.push_back(s->name);
            }
        }

        out << "static const struct {\n"
               "    const char *name;\n"
               "    State *(*body)(void);\n"
               "} START_RULES[] = {\n";
        unsigned i = 0;
        for (const string &s : start_rules) {
            out << "    { .name = " << escape_string(s) << ", .body = startstate_" << i << "},\n";
            i++;
        }
        out << "};\n\n";
    }

    // Write out the invariant rules.
    {
        vector<string> invariants;
        for (const shared_ptr<Rule> r : model.rules) {
            if (auto i = dynamic_pointer_cast<const Invariant>(r)) {
                out << "static bool invariant_" << invariants.size() << "(const State *s) {\n"
                       "    // TODO\n"
                       "    return true;\n"
                       "}\n\n";
                invariants.push_back(i->name);
            }
        }

        out << "static const struct {\n"
               "    const char *name;\n"
               "    bool (*guard)(const State*);\n"
               "} INVARIANTS[] = {\n";
        unsigned i = 0;
        for (const string &n : invariants) {
            out << "    { .name = " << escape_string(n) << ", .guard = invariant_" << i << "},\n";
            i++;
        }
        out << "};\n\n";
    }

    // Write out the regular rules.
    {
        vector<string> rules;
        for (const shared_ptr<Rule> r : model.rules) {
            if (is_regular_rule(r)) {
                out << "static bool guard_" << rules.size() << "(const State *s) {\n"
                       "    //TODO\n"
                       "    return true;\n"
                       "}\n\n"
                       "static void rule_" << rules.size() << "(State *s) {\n"
                       "    // TODO\n"
                       "}\n\n";
                rules.push_back(r->name);
            }
        }

        out << "static const struct {\n"
               "    const char *name;\n"
               "    bool (*guard)(const State*);\n"
               "    void (*body)(State *);\n"
               "} RULES[] = {\n";
        unsigned i = 0;
        for (const string &n : rules) {
            out << "    { .name = " << escape_string(n) << ", .guard = guard_" <<
              i << ", .body = rule_" << i << "},\n";
            i++;
        }
        out << "};\n\n";
    }

    WRITE(main_c);

#undef WRITE

    return 0;

}
