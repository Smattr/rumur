#include <iostream>
#include <fstream>
#include <rumur/Model.h>
#include <rumur/output.h>
#include "resources.h"
#include <string>
#include <vector>

using namespace rumur;
using namespace std;

int rumur::output_checker(const string &path, const Model &model,
  const OutputOptions &options) {

    ofstream out(path);
    if (!out)
        return -1;

    unsigned name_counter = 0;

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
    for (const Rule *r : model.rules) {
        if (auto s = dynamic_cast<const StartState*>(r)) {
            out << "static State model_";
            if (s->name == "") {
                out << name_counter;
                start_rules.push_back(to_string(name_counter));
                name_counter++;
            } else {
                out << s->name;
                start_rules.push_back(s->name);
            }
            out << "() {\n"
                   "    State s;\n"
                   "    // TODO\n"
                   "    return s;\n"
                   "}\n\n";
        }
    }

    out << "static const std::array<std::function<State()>, "
      << start_rules.size() << "> START_RULES = {";
    for (const string &s : start_rules)
        out << "model_" << s << ",";
    out << "};\n";

#undef WRITE

    return 0;

}
