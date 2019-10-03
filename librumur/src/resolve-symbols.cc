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
#include <rumur/validate.h>
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

    // register all the enum members so they can be referenced later
    mpz_class index = 0;
    size_t id = e->unique_id + 1;
    for (const std::pair<std::string, location> &m : n.members) {
      auto cd = Ptr<ConstDecl>::make(m.first,
        Ptr<Number>::make(index, m.second), e, m.second);
      // assign this member a unique id so that referrers can use it if need be
      assert(id < e->unique_id_limit && "number of enum members exceeds what "
        "was expected");
      cd->unique_id = id;
      symtab.declare(m.first, cd);
      index++;
      id++;
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
    for (auto &p : n.parameters)
      dispatch(*p);
    if (n.return_type != nullptr)
      dispatch(*n.return_type);
    // register the function itself, even though its body has not yet been
    // resolved, in order to allow contained function calls to resolve to the
    // containing function, supporting recursion
    symtab.declare(n.name, Ptr<Function>::make(n));
    // only register the function parameters now, to avoid their names shadowing
    // anything that needs to be resolved during symbol resolution of another
    // parameter or the return type
    for (auto &p : n.parameters)
      symtab.declare(p->name, p);
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

    // running marker of offset in the global state data
    mpz_class offset = 0;

    /* whether we have not yet hit any problems that make offset calculation
     * impossible
     */
    bool ok = true;

    for (auto &d : n.decls) {
      dispatch(*d);

      /* if this was a variable declaration, we now know enough to determine its
       * offset in the global state data
       */
      if (ok) {
        if (auto v = dynamic_cast<VarDecl*>(d.get())) {

          /* If the declaration or one of its children does not validate, it is
           * unsafe to call width().
           */
          try {
            validate(*v);
          } catch (Error&) {
            /* Skip this and future offset calculations and assume our caller
             * will eventually discover the underlying reason when they call
             * validate_model.
             */
            ok = false;
          }

          if (ok) {
            v->offset = offset;
            offset += v->type->width();
          }
        }
      }

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
    if (n.type != nullptr) {
      // wrap symbol resolution within the type in a dummy scope to suppress any
      // declarations (primarily enum members) as these will be duplicated in
      // when we descend into decl below
      symtab.open_scope();
      dispatch(*n.type);
      symtab.close_scope();
    }
    if (n.from != nullptr)
      dispatch(*n.from);
    if (n.to != nullptr)
      dispatch(*n.to);
    if (n.step != nullptr)
      dispatch(*n.step);
    dispatch(*n.decl);

    symtab.declare(n.name, n.decl);
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

      n.referent = t;
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
