#include "location.hh"
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Property.h>
#include <rumur/Ptr.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/except.h>
#include <rumur/traverse.h>
#include <string>
#include <vector>

namespace rumur {

namespace {
/* A traversal pass that checks any return statements within a rule do not
 * have a trailing expression.
 */
class ReturnChecker : public ConstTraversal {

public:
  /* Avoid recursing into functions, that may have return statements with an
   * expression.
   */
  void visit_function(const Function &) final {}
  void visit_functioncall(const FunctionCall &) final {}
  void visit_procedurecall(const ProcedureCall &) final {}

  void visit_return(const Return &n) final {
    if (n.expr != nullptr)
      throw Error("return statement in rule or startstate returns a value",
                  n.loc);

    // No need to recurse into the return statement's child.
  }

  static void check(const Node &n) {
    ReturnChecker c;
    c.dispatch(n);
  }

  virtual ~ReturnChecker() = default;
};
} // namespace

Rule::Rule(const std::string &name_, const location &loc_)
    : Node(loc_), name(name_) {}

std::vector<Ptr<Rule>> Rule::flatten() const { return {Ptr<Rule>(clone())}; }

AliasRule::AliasRule(const std::vector<Ptr<AliasDecl>> &aliases_,
                     const std::vector<Ptr<Rule>> &rules_, const location &loc_)
    : Rule("", loc_), rules(rules_) {

  aliases = aliases_;
}

AliasRule *AliasRule::clone() const { return new AliasRule(*this); }

void AliasRule::visit(BaseTraversal &visitor) {
  visitor.visit_aliasrule(*this);
}

void AliasRule::visit(ConstBaseTraversal &visitor) const {
  visitor.visit_aliasrule(*this);
}

std::vector<Ptr<Rule>> AliasRule::flatten() const {
  std::vector<Ptr<Rule>> rs;
  for (const Ptr<Rule> &r : rules) {
    for (Ptr<Rule> &f : r->flatten()) {
      f->aliases.insert(f->aliases.begin(), aliases.begin(), aliases.end());
      rs.push_back(f);
    }
  }
  return rs;
}

SimpleRule::SimpleRule(const std::string &name_, const Ptr<Expr> &guard_,
                       const std::vector<Ptr<Decl>> &decls_,
                       const std::vector<Ptr<Stmt>> &body_,
                       const location &loc_)
    : Rule(name_, loc_), guard(guard_), decls(decls_), body(body_) {}

SimpleRule *SimpleRule::clone() const { return new SimpleRule(*this); }

void SimpleRule::validate() const { ReturnChecker::check(*this); }

void SimpleRule::visit(BaseTraversal &visitor) {
  visitor.visit_simplerule(*this);
}

void SimpleRule::visit(ConstBaseTraversal &visitor) const {
  visitor.visit_simplerule(*this);
}

StartState::StartState(const std::string &name_,
                       const std::vector<Ptr<Decl>> &decls_,
                       const std::vector<Ptr<Stmt>> &body_,
                       const location &loc_)
    : Rule(name_, loc_), decls(decls_), body(body_) {}

StartState *StartState::clone() const { return new StartState(*this); }

void StartState::validate() const { ReturnChecker::check(*this); }

void StartState::visit(BaseTraversal &visitor) {
  visitor.visit_startstate(*this);
}

void StartState::visit(ConstBaseTraversal &visitor) const {
  visitor.visit_startstate(*this);
}

PropertyRule::PropertyRule(const std::string &name_, const Property &property_,
                           const location &loc_)
    : Rule(name_, loc_), property(property_) {}

PropertyRule *PropertyRule::clone() const { return new PropertyRule(*this); }

void PropertyRule::visit(BaseTraversal &visitor) {
  visitor.visit_propertyrule(*this);
}

void PropertyRule::visit(ConstBaseTraversal &visitor) const {
  visitor.visit_propertyrule(*this);
}

Ruleset::Ruleset(const std::vector<Quantifier> &quantifiers_,
                 const std::vector<Ptr<Rule>> &rules_, const location &loc_)
    : Rule("", loc_), rules(rules_) {
  quantifiers = quantifiers_;
}

Ruleset *Ruleset::clone() const { return new Ruleset(*this); }

void Ruleset::validate() const {
  for (const Quantifier &q : quantifiers) {
    if (!q.constant())
      throw Error("non-constant quantifier expression as ruleset parameter",
                  q.loc);
  }
}

void Ruleset::visit(BaseTraversal &visitor) { visitor.visit_ruleset(*this); }

void Ruleset::visit(ConstBaseTraversal &visitor) const {
  visitor.visit_ruleset(*this);
}

std::vector<Ptr<Rule>> Ruleset::flatten() const {
  std::vector<Ptr<Rule>> rs;
  for (const Ptr<Rule> &r : rules) {
    for (Ptr<Rule> &f : r->flatten()) {
      for (const Quantifier &q : quantifiers)
        f->quantifiers.push_back(q);
      rs.push_back(f);
    }
  }
  return rs;
}

} // namespace rumur
