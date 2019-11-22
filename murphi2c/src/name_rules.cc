#include <cstddef>
#include "name_rules.h"
#include <rumur/rumur.h>
#include <string>

using namespace rumur;

namespace {

class RuleNamer : public Traversal {

 private:
  size_t index = 0;

  void name(Rule &n) {
    if (n.name == "") {
      n.name = std::to_string(index);
      index++;
    }
  }

 public:
  void visit_aliasrule(AliasRule &n) final {
    name(n);
    for (Ptr<Rule> &r : n.rules) {
      dispatch(*r);
    }
  }

  void visit_propertyrule(PropertyRule &n) final {
    name(n);
  }

  void visit_ruleset(Ruleset &n) final {
    name(n);
    for (Ptr<Rule> &r : n.rules) {
      dispatch(*r);
    }
  }

  void visit_simplerule(SimpleRule &n) final {
    name(n);
  }
};

}

void name_rules(Node &n) {
  RuleNamer r;
  r.dispatch(n);
}
