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

    WRITE(header_cc);

    WRITE(State_cc);

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
      << start_rules.size() << "> START_RULES = {";

    unsigned i = 0;
    for (const string &s : start_rules) {
        out << "std::make_pair(" << escape_string(s) << ", startstate_" << i << "),";
        i++;
    }
    out << "};\n\n";

    WRITE(main_cc);

#undef WRITE

    return 0;

}
