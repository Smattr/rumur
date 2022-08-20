#include "check.h"
#include "is_one_step.h"
#include <cassert>
#include <cstddef>
#include <rumur/rumur.h>

using namespace rumur;

namespace {

class Checker : public ConstTraversal {

private:
  bool is_tail = false;

  std::vector<const Quantifier *> params;
  ///< parameters in the ruleset context we are currently within

public:
  void visit_aliasdecl(const AliasDecl &n) final {
    // I think Uclid5 has nothing that could reasonably implement AliasDecl…?
    throw Error("Uclid5 has no equivalent of alias declarations", n.loc);
  }

  void visit_aliasrule(const AliasRule &n) final {
    // I think Uclid5 has nothing that could reasonably implement AliasRule…?
    throw Error("Uclid5 has no equivalent of alias rules", n.loc);
  }

  void visit_aliasstmt(const AliasStmt &n) final {
    // I think Uclid5 has nothing that could reasonably implement AliasStmt…?
    throw Error("Uclid5 has no equivalent of alias statements", n.loc);
  }

  void visit_clear(const Clear &n) final {
    const Ptr<TypeExpr> type = n.rhs->type();

    if (!type->is_simple())
      throw Error("Clear of complex types is not supported", n.loc);
  }

  void visit_div(const Div &n) final {
    throw Error("Uclid5 has no equivalent of the division operator", n.loc);
  }

  void visit_exists(const Exists &n) final {
    // we could support this, but it seems rarely used so reject it for now
    if (n.quantifier.type == nullptr && !is_one_step(n.quantifier.step))
      throw Error("exists with a non-1 stride are not supported", n.loc);
  }

  void visit_forall(const Forall &n) final {
    // we could support this, but it seems rarely used so reject it for now
    if (n.quantifier.type == nullptr && !is_one_step(n.quantifier.step))
      throw Error("forall with a non-1 stride are not supported", n.loc);
  }

  void visit_isundefined(const IsUndefined &n) final {
    throw Error("Uclid5 has no equivalent of the isundefined operator", n.loc);
  }

  void visit_function(const Function &n) final {
    assert(!is_tail && "incorrect tracking of tail statements");

    for (const Ptr<VarDecl> &p : n.parameters)
      p->visit(*this);
    if (n.return_type != nullptr)
      n.return_type->visit(*this);
    for (const Ptr<Decl> &d : n.decls)
      d->visit(*this);

    // descend while only considering the last statement a tail
    for (size_t i = 0; i + 1 < n.body.size(); ++i)
      n.body[i]->visit(*this);
    is_tail = true;
    if (!n.body.empty())
      n.body.back()->visit(*this);

    // restore default is_tail state
    is_tail = false;
  }

  void visit_for(const For &n) final {
    n.quantifier.visit(*this);

    // save current is_tail state
    bool old_is_tail = is_tail;
    is_tail = false;

    for (size_t i = 0; i + 1 < n.body.size(); ++i)
      n.body[i]->visit(*this);

    // restore is_tail for the last statement in the block
    is_tail = old_is_tail;

    if (!n.body.empty())
      n.body.back()->visit(*this);
  }

  void visit_ifclause(const IfClause &n) final {
    if (n.condition != nullptr)
      n.condition->visit(*this);

    // save current is_tail state
    bool old_is_tail = is_tail;
    is_tail = false;

    for (size_t i = 0; i + 1 < n.body.size(); ++i)
      n.body[i]->visit(*this);

    // restore is_tail for the last statement in the block
    is_tail = old_is_tail;

    if (!n.body.empty())
      n.body.back()->visit(*this);
  }

  void visit_lsh(const Lsh &n) final {
    // TODO: technically we could implement this as a Uclid5 function. However,
    // it is a little awkward because Uclid5 does not support generic functions
    // so we would have to detect which types << is used with and emit a
    // function for each of these.
    throw Error("Uclid5 has no equivalent of the left shift operator", n.loc);
  }

  void visit_mod(const Mod &n) final {
    throw Error("Uclid5 has no equivalent of the modulo operator", n.loc);
  }

  void visit_propertyrule(const PropertyRule &n) final {
    if (n.property.category == Property::COVER)
      throw Error("cover properties have no LTL equivalent in Uclid5", n.loc);

    // forall quantifiers outside `G(F(…))` does not work in Uclid5
    if (!params.empty() && n.property.category == Property::LIVENESS)
      throw Error("liveness properties within rulesets cannot be translated to "
                  "Uclid5",
                  n.loc);

    for (const Quantifier *q : params) {
      if (q->type == nullptr && !is_one_step(q->step))
        throw Error("properties within rulesets using quantifiers with non-1 "
                    "steps are not supported",
                    q->loc);
    }

    n.property.visit(*this);
  }

  void visit_propertystmt(const PropertyStmt &n) final {

    if (n.property.category == Property::COVER)
      throw Error("Uclid5 has no equivalent of cover statements", n.loc);

    if (n.property.category == Property::LIVENESS)
      throw Error("Ucild5 has no equivalent of liveness statements", n.loc);

    n.property.visit(*this);
  }

  void visit_put(const Put &n) final {
    throw Error("Uclid5 has no equivalent of put statements except print, "
                "which is only permitted in control blocks",
                n.loc);
  }

  void visit_return(const Return &n) final {

    // there seems to be no way to return early from a Uclid5 procedure
    if (!is_tail)
      throw Error("early return statements are not supported", n.loc);

    if (n.expr != nullptr)
      n.expr->visit(*this);
  }

  void visit_rsh(const Rsh &n) final {
    // TODO: technically we could implement this as a Uclid5 function. However,
    // it is a little awkward because Uclid5 does not support generic functions
    // so we would have to detect which types >> is used with and emit a
    // function for each of these.
    throw Error("Uclid5 has no equivalent of the right shift operator", n.loc);
  }

  void visit_ruleset(const Ruleset &n) final {

    for (const Quantifier &q : n.quantifiers) {
      q.visit(*this);
      // make each parameter visible to following children as we descend
      params.push_back(&q);
    }

    for (const Ptr<Rule> &r : n.rules)
      r->visit(*this);

    assert(params.size() >= n.quantifiers.size() &&
           "ruleset parameter management out of sync");
    for (size_t i = 0; i < n.quantifiers.size(); ++i) {
      size_t j __attribute__((unused)) =
          params.size() - n.quantifiers.size() + i;
      assert(params[j] == &n.quantifiers[i] &&
             "ruleset parameter management out of sync");
    }

    // remove the parameters as we ascend
    params.resize(params.size() - n.quantifiers.size());
  }

  void visit_simplerule(const SimpleRule &n) final {
    assert(!is_tail && "incorrect trackin of tail statements");

    for (const Quantifier &q : n.quantifiers)
      q.visit(*this);
    for (const Ptr<AliasDecl> &a : n.aliases)
      a->visit(*this);
    if (n.guard != nullptr)
      n.guard->visit(*this);
    for (const Ptr<Decl> &d : n.decls)
      d->visit(*this);

    // descend while only considering the last statement a tail
    for (size_t i = 0; i + 1 < n.body.size(); ++i)
      n.body[i]->visit(*this);
    is_tail = true;
    if (!n.body.empty())
      n.body.back()->visit(*this);

    // restore default is_tail state
    is_tail = false;
  }

  void visit_startstate(const StartState &n) final {
    assert(!is_tail && "incorrect trackin of tail statements");

    for (const Quantifier &q : n.quantifiers)
      q.visit(*this);
    for (const Ptr<AliasDecl> &a : n.aliases)
      a->visit(*this);
    for (const Ptr<Decl> &d : n.decls)
      d->visit(*this);

    // descend while only considering the last statement a tail
    for (size_t i = 0; i + 1 < n.body.size(); ++i)
      n.body[i]->visit(*this);
    is_tail = true;
    if (!n.body.empty())
      n.body.back()->visit(*this);

    // restore default is_tail state
    is_tail = false;
  }

  void visit_switchcase(const SwitchCase &n) final {
    for (const Ptr<Expr> &m : n.matches)
      m->visit(*this);

    // save current is_tail state
    bool old_is_tail = is_tail;
    is_tail = false;

    for (size_t i = 0; i + 1 < n.body.size(); ++i)
      n.body[i]->visit(*this);

    // restore is_tail for the last statement in the block
    is_tail = old_is_tail;

    if (!n.body.empty())
      n.body.back()->visit(*this);
  }

  void visit_while(const While &n) final {
    n.condition->visit(*this);

    // save current is_tail state
    bool old_is_tail = is_tail;
    is_tail = false;

    for (size_t i = 0; i + 1 < n.body.size(); ++i)
      n.body[i]->visit(*this);

    // restore is_tail for the last statement in the block
    is_tail = old_is_tail;

    if (!n.body.empty())
      n.body.back()->visit(*this);
  }
};

} // namespace

void check(const Node &n) {
  Checker c;
  n.visit(c);
}
