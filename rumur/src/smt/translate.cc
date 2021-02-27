#include "translate.h"
#include "../../../common/isa.h"
#include "../options.h"
#include "except.h"
#include "logic.h"
#include <cstddef>
#include <locale>
#include <rumur/rumur.h>
#include <sstream>
#include <string>

using namespace rumur;

namespace smt {

namespace {
class Translator : public ConstExprTraversal {

private:
  std::ostringstream buffer;

public:
  std::string str() const { return buffer.str(); }

  Translator &operator<<(const std::string &s) {
    buffer << s;
    return *this;
  }

  Translator &operator<<(const Expr &e) {
    dispatch(e);
    return *this;
  }

  void visit_add(const Add &n) {
    *this << "(" << add() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_and(const And &n) {
    *this << "(and " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_band(const Band &n) {
    *this << "(" << band() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_bnot(const Bnot &n) {
    *this << "(" << bnot() << " " << *n.rhs << ")";
  }

  void visit_bor(const Bor &n) {
    *this << "(" << bor() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_element(const Element &n) {
    *this << "(select " << *n.array << " " << *n.index << ")";
  }

  void visit_exprid(const ExprID &n) {
    *this << mangle(n.id, n.value->unique_id);
  }

  void visit_eq(const Eq &n) {
    *this << "(= " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_exists(const Exists &n) {
    translate_quantified(n.quantifier, *n.expr, false);
  }

  void visit_div(const Div &n) {
    *this << "(" << div() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_field(const Field &n) {
    // the record type that forms the root of this expression will have
    // previously been defined as a synthesised type (see define-records.cc)

    // determine the (mangled) SMT name the root was given
    const Ptr<TypeExpr> type = n.record->type()->resolve();
    const std::string root = mangle("", type->unique_id);

    // we can now compute the accessor for this field (see define-records.cc)
    const std::string getter = root + "_" + n.field;

    // now construct the SMT expression
    *this << "(" << getter << " " << *n.record << ")";
  }

  void visit_forall(const Forall &n) {
    translate_quantified(n.quantifier, *n.expr, true);
  }

  void visit_functioncall(const FunctionCall &n) { throw Unsupported(n); }

  void visit_geq(const Geq &n) {
    *this << "(" << geq() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_gt(const Gt &n) {
    *this << "(" << gt() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_implication(const Implication &n) {
    *this << "(=> " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_isundefined(const IsUndefined &n) { throw Unsupported(n); }

  void visit_leq(const Leq &n) {
    *this << "(" << leq() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_lsh(const Lsh &n) {
    *this << "(" << lsh() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_lt(const Lt &n) {
    *this << "(" << lt() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_mod(const Mod &n) {
    *this << "(" << mod() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_mul(const Mul &n) {
    *this << "(" << mul() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_negative(const Negative &n) {
    *this << "(" << neg() << " " << *n.rhs << ")";
  }

  void visit_neq(const Neq &n) {
    *this << "(not (= " << *n.lhs << " " << *n.rhs << "))";
  }

  void visit_number(const Number &n) { *this << numeric_literal(n.value); }

  void visit_not(const Not &n) { *this << "(not " << *n.rhs << ")"; }

  void visit_or(const Or &n) {
    *this << "(or " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_rsh(const Rsh &n) {
    *this << "(" << rsh() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_sub(const Sub &n) {
    *this << "(" << sub() << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_ternary(const Ternary &n) {
    *this << "(ite " << *n.cond << " " << *n.lhs << " " << *n.rhs << ")";
  }

  void visit_xor(const Xor &n) {
    *this << "(" << bxor() << " " << *n.lhs << " " << *n.rhs << ")";
  }

private:
  void translate_quantified(const Quantifier &q, const Expr &e, bool forall) {

    // find a name for the quantified variable
    const std::string qname = mangle(q.decl->name, q.decl->unique_id);
    const std::string qtype = integer_type();

    // determine the parts of the expression we will construct that depend on
    // forall
    const std::string binder  = forall ? "forall" : "exists";
    const std::string op      = forall ? "or"     : "and";
    const std::string lb_rel  = forall ? lt()     : geq();
    const std::string ub_rel1 = forall ? gt()     : leq();
    const std::string ub_rel2 = forall ? geq()    : lt();
    const std::string step_o  = forall ? "(not "  : "";
    const std::string step_c  = forall ? ")"      : "";

    // “∀q.”/“∃q.”
    *this << "(" << binder << " ((" << qname << " " << qtype << ")) (" << op;

    // “q < lb”/“q ≥ lb”
    *this << " (" << lb_rel << " " << qname << " ";
    if (q.type != nullptr) {
      const Ptr<TypeExpr> t = q.type->resolve();

      if (isa<Enum>(t) || isa<Scalarset>(t)) {
        *this << numeric_literal(0);
      } else {
        assert(isa<Range>(t) &&
               "non-(range|enum|scalarset) variable in quantifier");
        const Range &r = dynamic_cast<const Range &>(*t);
        assert(r.min != nullptr && "unbounded range type");
        *this << *r.min;
      }
    } else {
      assert(q.from != nullptr &&
             "quantified variable has no type and also no lower bound");
      *this << *q.from;
    }
    *this << ")";

    // or/and “q > ub”/“q ≤ ub”
    *this << " (";
    if (q.type != nullptr) {
      const Ptr<TypeExpr> t = q.type->resolve();

      assert((isa<Enum>(t) || isa<Range>(t) || isa<Scalarset>(t)) &&
             "non-(range|enum|scalarset) variable in quantifier");

      if (auto n = dynamic_cast<const Enum *>(t.get())) {
        size_t s = n->members.size();
        *this << ub_rel2 << " " << qname << " " << numeric_literal(s);

      } else if (auto r = dynamic_cast<const Range *>(t.get())) {
        assert(r->max != nullptr && "unbounded range type");
        *this << ub_rel1 << " " << qname << " " << *r->max;

      } else {
        auto s = dynamic_cast<const Scalarset &>(*t);
        *this << ub_rel2 << " " << qname << " " << *s.bound;
      }
    } else {
      assert(q.to != nullptr &&
             "quantified variable has no type and also no upper bound");
      *this << ub_rel1 << " " << qname << " " << *q.to;
    }
    *this << ")";

    // or/and “!∃i. q = lb + i * step”/“∃i. q = lb + i * step”
    const std::string iname = qname + "_iteration";
    *this << " " << step_o << "(exists ((" << iname << " " << qtype
          << ")) (= " << qname << " (" << add() << " ";
    if (q.type != nullptr) {
      const Ptr<TypeExpr> t = q.type->resolve();

      if (isa<Enum>(t) || isa<Scalarset>(t)) {
        *this << numeric_literal(0);
      } else {
        assert(isa<Range>(t) &&
               "non-(range|enum|scalarset) variable in quantifier");
        const Range &r = dynamic_cast<const Range &>(*t);
        assert(r.min != nullptr && "unbounded range type");
        *this << *r.min;
      }
    } else {
      *this << *q.from;
    }
    *this << " (" << mul() << " " << iname << " ";
    if (q.step == nullptr) {
      *this << numeric_literal(1);
    } else {
      *this << *q.step;
    }
    *this << "))))" << step_c;

    // finally the enclosed expression itself
    *this << " " << e << "))";
  }
};
} // namespace

std::string translate(const Expr &expr) {
  Translator t;
  t.dispatch(expr);
  return t.str();
}

static std::string lower(const std::string &s) {
  std::string s1;
  for (char c : s)
    s1 += tolower(c);
  return s1;
}

std::string mangle(const std::string &s, size_t id) {

  // if you're debugging a bad translation to SMT, you can change this to `true`
  // to get the Murphi name of a symbol output as a comment in the SMT problem
  const std::string prefix = false ? "; " + s + ":\n" : "";

  const std::string l = lower(s);

  // if this is a boolean literal, the solver already knows of it
  if (l == "true" || l == "false")
    return prefix + l;

  // if this is the boolean type, the solver already knows of it
  if (l == "boolean")
    return prefix + "Bool";

  // otherwise synthesise a node-unique name for this
  return prefix + "s" + std::to_string(id);
}

} // namespace smt
