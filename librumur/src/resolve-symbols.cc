#include <cstddef>
#include <gmpxx.h>
#include "location.hh"
#include <memory>
#include <rumur/Boolean.h>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Model.h>
#include <rumur/Number.h>
#include <rumur/Ptr.h>
#include <rumur/resolve-symbols.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/Symtab.h>
#include <rumur/traverse.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>

namespace rumur {

namespace {

class Resolver : public Traversal {

 private:
  Symtab symtab;

 public:
  Resolver() {

    // Open a global scope
    symtab.open_scope();

    // Teach the symbol table the built ins
    auto td = Ptr<TypeDecl>::make("boolean", Boolean, location());
    symtab.declare("boolean", td);
    mpz_class index = 0;
    for (const std::pair<std::string, location> &m : Boolean->members) {
      symtab.declare(m.first, Ptr<ConstDecl>::make("boolean",
        Ptr<Number>::make(index, location()), Boolean, location()));
      index++;
    }
  }

  void visit_aliasrule(AliasRule &n) final {
    symtab.open_scope();
    for (auto &a : n.aliases) {
      dispatch(*a);
      symtab.declare(a->name, a);
    }
    for (auto &r : n.rules)
      dispatch(*r);
    symtab.close_scope();
  }

  void visit_aliasstmt(AliasStmt &n) final {
    symtab.open_scope();
    for (auto &a : n.aliases) {
      dispatch(*a);
      symtab.declare(a->name, a);
    }
    for (auto &s : n.body)
      dispatch(*s);
    symtab.close_scope();
  }

  void visit_enum(Enum &n) final {
    auto e = Ptr<Enum>::make(n);

    //* Register all the enum members so they can be referenced later.
    mpz_class index = 0;
    for (const std::pair<std::string, location> &m : n.members) {
      auto cd = Ptr<ConstDecl>::make(m.first,
        Ptr<Number>::make(index, m.second), e, m.second);
      symtab.declare(m.first, cd);
      index++;
    }
  }

  void visit_exists(Exists &n) final {
    symtab.open_scope();
    dispatch(n.quantifier);
    dispatch(*n.expr);
    symtab.close_scope();
  }

  void visit_exprid(ExprID &n) final {
    if (n.value == nullptr) {
      // This reference is unresolved

      Ptr<ExprDecl> d = symtab.lookup<ExprDecl>(n.id, n.loc);
      if (d == nullptr)
        throw Error("unknown symbol \"" + n.id + "\"", n.loc);

      n.value = d;
    }
  }

  void visit_for(For &n) final {
    symtab.open_scope();
    dispatch(n.quantifier);
    for (auto &s : n.body)
      dispatch(*s);
    symtab.close_scope();
  }

  void visit_forall(Forall &n) final {
    symtab.open_scope();
    dispatch(n.quantifier);
    dispatch(*n.expr);
    symtab.close_scope();
  }

  void visit_function(Function &n) final {
    symtab.open_scope();
    for (auto &p : n.parameters) {
      dispatch(*p);
      symtab.declare(p->name, p);
    }
    if (n.return_type != nullptr)
      dispatch(*n.return_type);
    for (auto &d : n.decls) {
      dispatch(*d);
      symtab.declare(d->name, d);
    }
    for (auto &s : n.body)
      dispatch(*s);
    symtab.close_scope();
  }

  void visit_functioncall(FunctionCall &n) final {
    if (n.function == nullptr) {
      // This reference is unresolved

      Ptr<Function> f = symtab.lookup<Function>(n.name, n.loc);
      if (f == nullptr)
        throw Error("unknown function call \"" + n.name + "\"", n.loc);

      n.function = f;
    }
    for (auto &a : n.arguments)
      dispatch(*a);
  }

  void visit_model(Model &n) final {
    for (auto &d : n.decls) {
      dispatch(*d);
      symtab.declare(d->name, d);
    }
    for (auto &f : n.functions) {
      dispatch(*f);
      symtab.declare(f->name, f);
    }
    for (auto &r : n.rules)
      dispatch(*r);
  }

  void visit_quantifier(Quantifier &n) final {
    if (n.type != nullptr)
      dispatch(*n.type);
    if (n.from != nullptr)
      dispatch(*n.from);
    if (n.to != nullptr)
      dispatch(*n.to);
    if (n.step != nullptr)
      dispatch(*n.step);

    /* We need to register the quantifier variable to be resolvable within this
     * scope. However it may not have a proper type. To cope with this, we
     * construct a type on the fly here if necessary.
     */
    Ptr<TypeExpr> t;
    if (n.type == nullptr) {
      t = Ptr<Range>::make(nullptr, nullptr, location());
    } else {
      t = n.type;
    }
    symtab.declare(n.name, Ptr<VarDecl>::make(n.name, t, n.loc));
  }

  void visit_ruleset(Ruleset &n) final {
    symtab.open_scope();
    for (Quantifier &q : n.quantifiers)
      dispatch(q);
    for (auto &r : n.rules)
      dispatch(*r);
    symtab.close_scope();
  }

  void visit_simplerule(SimpleRule &n) final {
    symtab.open_scope();
    for (Quantifier &q : n.quantifiers)
      dispatch(q);
    if (n.guard != nullptr)
      dispatch(*n.guard);
    for (auto &d : n.decls) {
      dispatch(*d);
      symtab.declare(d->name, d);
    }
    for (auto &s : n.body)
      dispatch(*s);
    symtab.close_scope();
  }

  void visit_startstate(StartState &n) final {
    symtab.open_scope();
    for (Quantifier &q : n.quantifiers)
      dispatch(q);
    for (auto &d : n.decls) {
      dispatch(*d);
      symtab.declare(d->name, d);
    }
    for (auto &s : n.body)
      dispatch(*s);
    symtab.close_scope();
  }

  void visit_typeexprid(TypeExprID &n) final {
    if (n.referent == nullptr) {
      // This reference is unresolved

      Ptr<TypeDecl> t = symtab.lookup<TypeDecl>(n.name, n.loc);
      if (t == nullptr)
        throw Error("unknown type symbol \"" + n.name + "\"", n.loc);

      n.referent = t->value;
    }
  }

  virtual ~Resolver() = default;
};

}

void resolve_symbols(Model &m) {
  Resolver r;
  r.dispatch(m);
}

}
