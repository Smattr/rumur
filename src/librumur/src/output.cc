#include <iostream>
#include <fstream>
#include <memory>
#include <rumur/Model.h>
#include <rumur/output.h>
#include "resources.h"
#include <string>
#include <utility>
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

static const vector<pair<string, string>> INCLUDES = {
#define RES(r) make_pair(#r ".cc", string((const char*)resources_##r##_cc, (size_t)resources_##r##_cc_len))
    RES(State),
    RES(Rule),
    RES(header),
    RES(main),
#undef RES
};

int rumur::output_includes(const string &path) {
    for (auto [p, data] : INCLUDES) {
        ofstream out(path + "/" + p);
        if (!out)
            return -1;

        out << data;
    }
    return 0;
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

    out
      << "static constexpr bool OVERFLOW_CHECKS_ENABLED = " <<
      (options.overflow_checks ? "true" : "false") << ";\n"

      << "static constexpr size_t STATE_SIZE_BITS = " << model.size_bits()
      << ";\n"

      << "\n"

      << "#include \"Rule.cc\"\n"
      << "#include \"State.cc\"\n"
      << "#include \"header.cc\"\n"

      << "\n";

    // TODO: rewrite the following into C++17

    // Write out constants and type declarations.
    for (const shared_ptr<Decl> d : model.decls)
        d->define(out);

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
                s->generate_rule(out);
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

    out << "#include \"main.c\"\n";

    return 0;

}
