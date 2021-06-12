#include <cstddef>
#include <rumur/Node.h>
#include <rumur/Ptr.h>
#include <rumur/Rule.h>
#include <rumur/sanitise_rule_names.h>
#include <rumur/traverse.h>
#include <string>
#include <unordered_set>

using namespace rumur;

namespace {

class RuleNamer : public Traversal {

private:
  size_t index = 0;
  std::unordered_set<std::string> used; // names already taken

  void name(Rule &n, const std::string &prefix) {

    std::string replacement = n.name;

    // name by index if there is no name
    if (replacement == "") {
      replacement = prefix + std::to_string(index);
      index++;
    }

    // remove non-symbol characters
    for (char &c : replacement) {
      if (!(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z') &&
          !(c >= '0' && c <= '9')) {
        c = '_';
      }
    }

    std::string candidate = replacement;
    while (!used.insert(candidate).second) {
      candidate = replacement + std::to_string(index);
      index++;
    }

    n.name = candidate;
  }

public:
  void visit_aliasrule(AliasRule &n) final {
    name(n, "alias");
    for (Ptr<Rule> &r : n.rules) {
      dispatch(*r);
    }
  }

  void visit_propertyrule(PropertyRule &n) final { name(n, "property"); }

  void visit_ruleset(Ruleset &n) final {
    name(n, "ruleset");
    for (Ptr<Rule> &r : n.rules) {
      dispatch(*r);
    }
  }

  void visit_simplerule(SimpleRule &n) final { name(n, "rule"); }

  void visit_startstate(StartState &n) final { name(n, "startstate"); }
};

} // namespace

void rumur::sanitise_rule_names(Node &n) {
  RuleNamer r;
  r.dispatch(n);
}
