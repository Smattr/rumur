#include "check.h"
#include <cstddef>
#include <cassert>
#include <rumur/rumur.h>

using namespace rumur;

namespace {

class Checker : public ConstTraversal {

private:
  bool is_tail = false;

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
