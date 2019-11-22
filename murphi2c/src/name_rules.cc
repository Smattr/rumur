#include <cstddef>
#include "name_rules.h"
#include <rumur/rumur.h>
#include <string>

using namespace rumur;

namespace {

class RuleNamer : public Traversal {

 private:
  size_t index = 0;

  void name(Rule &n, const std::string &prefix) {

    // name by index if there is no name
    if (n.name == "") {
      n.name = prefix + std::to_string(index);
      index++;
      return;
    }

    // otherwise remove non-symbol characters
    for (char &c : n.name) {
      if (!(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z') &&
          !(c >= '0' && c <= '9')) {
        c = '_';
      }
    }
  }

 public:
  void visit_aliasrule(AliasRule &n) final {
    name(n, "alias");
    for (Ptr<Rule> &r : n.rules) {
      dispatch(*r);
    }
  }

  void visit_propertyrule(PropertyRule &n) final {
    name(n, "property");
  }

  void visit_ruleset(Ruleset &n) final {
    name(n, "ruleset");
    for (Ptr<Rule> &r : n.rules) {
      dispatch(*r);
    }
  }

  void visit_simplerule(SimpleRule &n) final {
    name(n, "rule");
  }
};

}

void name_rules(Node &n) {
  RuleNamer r;
  r.dispatch(n);
}
