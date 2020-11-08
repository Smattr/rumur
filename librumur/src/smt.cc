#include <cstddef>
#include <gmpxx.h>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/Number.h>
#include <rumur/smt.h>
#include <rumur/Stmt.h>
#include <rumur/traverse.h>
#include <sstream>
#include <string>
#include "utils.h"

namespace rumur {

SMTContext::SMTContext() { }

SMTContext::SMTContext(bool prefer_bitvectors_, size_t bitvector_width_)
 : prefer_bitvectors(prefer_bitvectors_), bitvector_width(bitvector_width_) { }

std::string SMTContext::register_symbol(size_t id) {
  // invent a new symbol and map this ID to it
  std::string s = make_symbol();
  scope.emplace_back(id, s);
  return s;
}

std::string SMTContext::lookup_symbol(size_t id, const Node &origin) const {

  // lookup the symbol in enclosing scopes from innermost to outermost
  for (auto it = scope.rbegin(); it != scope.rend(); ++it) {
    if (it->first == id)
      return it->second;
  }

  // we expect any symbol encountered in a well-formed AST to be associated with
  // a previously encountered definition
  throw Error("unknown symbol encountered; applying SMT translation to an "
    "unvalidated AST?", origin.loc);
}

std::string SMTContext::make_type(const std::string &type) {

  // create a name for this type
  const std::string name = make_symbol();

  // define the type
  content << "(define-sort " << name << " () " << type << ")\n";

  return name;
}

std::string SMTContext::make_type(size_t id, const std::string &type) {

  // create a name for this type
  const std::string name = register_symbol(id);

  // define the type
  content << "(define-sort " << name << " () " << type << ")\n";

  return name;
}

std::string SMTContext::make_symbol() {
  return "s" + std::to_string(counter++);
}

std::string SMTContext::add(const Node&) const {
  return prefer_bitvectors ? "bvadd" : "+";
}

std::string SMTContext::band(const Node &origin) const {
  if (prefer_bitvectors)
    return "bvand";
  throw Error("SMT translation involving bitwise AND is only supported when "
    "using bitvector representations", origin.loc);
}

std::string SMTContext::bnot(const Node &origin) const {
  if (prefer_bitvectors)
    return "bvnot";
  throw Error("SMT translation involving bitwise NOT is only supported when "
    "using bitvector representations", origin.loc);
}

std::string SMTContext::bor(const Node &origin) const {
  if (prefer_bitvectors)
    return "bvor";
  throw Error("SMT translation involving bitwise OR is only supported when "
    "using bitvector representations", origin.loc);
}

std::string SMTContext::bxor(const Node &origin) const {
  if (prefer_bitvectors)
    return "bvxor";
  throw Error("SMT translation involving bitwise XOR is only supported when "
    "using bitvector representations", origin.loc);
}

std::string SMTContext::div(const Node&) const {
  // solvers like CVC4 may fail with an error when given "div" but just ignore
  // this for now
  return prefer_bitvectors ? "bvsdiv" : "div";
}

std::string SMTContext::geq(const Node&) const {
  return prefer_bitvectors ? "bvsge" : ">=";
}

std::string SMTContext::gt(const Node&) const {
  return prefer_bitvectors ? "bvsgt" : ">";
}

std::string SMTContext::leq(const Node&) const {
  return prefer_bitvectors ? "bvsle" : "<=";
}

std::string SMTContext::lsh(const Node &origin) const {
  if (prefer_bitvectors)
    return "bvshl";
  throw Error("SMT translation involving left shift is only supported when "
    "using bitvector representations", origin.loc);
}

std::string SMTContext::lt(const Node&) const {
  return prefer_bitvectors ? "bvslt" : "<";
}

std::string SMTContext::mod(const Node&) const {
  return prefer_bitvectors ? "bvsmod" : "mod";
}

std::string SMTContext::mul(const Node&) const {
  return prefer_bitvectors ? "bvmul" : "*";
}

std::string SMTContext::neg(const Node&) const {
  return prefer_bitvectors ? "bvneg" : "-";
}

std::string SMTContext::rsh(const Node &origin) const {
  if (prefer_bitvectors)
    return "bvashr";
  throw Error("SMT translation involving right shift is only supported when "
    "using bitvector representations", origin.loc);
}

std::string SMTContext::sub(const Node&) const {
  return prefer_bitvectors ? "bvsub" : "-";
}

std::string SMTContext::numeric_literal(const mpz_class &value,
    const Number &origin) const {

  if (value < 0)
    return "(" + neg(origin) + " " + numeric_literal(-value, origin) + ")";

  if (prefer_bitvectors) {
    return "(_ bv" + value.get_str() + " " + std::to_string(bitvector_width)
      + ")";
  }

  return value.get_str();
}

SMTContext &SMTContext::operator<<(const std::string &s) {
  content << s;
  return *this;
}

// unravel an lvalue to its leftmost component
static const Expr &get_stump(const Expr &lvalue) {

  // an lvalue can only be an identifier, record field, or array element (see
  // parser.yy)

  if (auto i = dynamic_cast<const Field*>(&lvalue)) {
    // do we need to keep unraveling?
    if (!isa<ExprID>(i->record))
      return get_stump(*i->record);
    return lvalue;
  }

  if (auto i = dynamic_cast<const Element*>(&lvalue)) {
    // do we need to keep unraveling?
    if (!isa<ExprID>(i->array))
      return get_stump(*i->array);
    return lvalue;
  }

  return lvalue;
}

// retrieve the originating ID of an lvalue
static const ExprID &get_root(const Expr &lvalue) {

  // an lvalue can only be an identifier, record field, or array element (see
  // parser.yy)

  if (auto i = dynamic_cast<const Field*>(&lvalue))
    return get_root(*i->record);

  if (auto i = dynamic_cast<const Element*>(&lvalue))
    return get_root(*i->array);

  if (auto i = dynamic_cast<const ExprID*>(&lvalue))
    return *i;

  throw Error("expression in lvalue is not an identifier, record field, or "
    "array element", lvalue.loc);
}

// name mangling for a type guard function
static std::string get_type_guard(const std::string &type) {
  return "in_type_" + type;
}

namespace { class Translator : public ConstTraversal {

 private:
  SMTContext &ctxt;

  // buffer used for returning a string from a visitation function (see eval())
  std::ostringstream ret;

 public:
  Translator(SMTContext &ctxt_): ctxt(ctxt_) { }

  std::string str() const {
    return ret.str();
  }

  void visit_add(const Add &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(" << ctxt.add(n) << " " << lhs << " " << rhs << ")";
  }

  void visit_and(const And &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(and " << lhs << " " << rhs << ")";
  }

  void visit_assignment(const Assignment &n) {

    // Translate the RHS, that we need to evaluate *before* the LHS. The RHS may
    // contain an ID that is also the LHS, but the RHS needs to see the SMT
    // symbol of the state before the assignment.
    const std::string rhs = eval(*n.rhs);

    // find the root expression whose value we need to update
    const Expr &stump = get_stump(*n.lhs);

    // determine how to express an update to this entity

    std::ostringstream buf;

    if (auto i = dynamic_cast<const ExprID*>(&stump)) {
      buf << rhs;

    } else if (auto f = dynamic_cast<const Field*>(&stump)) {
      buf << "(mk_TODO ...";

    } else if (auto e = dynamic_cast<const Element*>(&stump)) {
      const std::string array = eval(*e->array);
      const std::string index = eval(*e->index);
      buf << "(store " << array << " " << index << " " << rhs << ")";

    } else {
      throw Error("expression in lvalue is not an identifier, record field, or "
        "array element", n.lhs->loc);
    }

    // find the left hand side of the stump
    const ExprID &root = get_root(stump);

    // invent a new name to store the updated value
    const std::string fresh = ctxt.register_symbol(root.value->unique_id);

    ctxt << "(assert (= " << fresh << " " << buf.str() << "))\n";
  }

  void visit_band(const Band &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(" << ctxt.band(n) << " " << lhs << " " << rhs << ")";
  }

  void visit_bnot(const Bnot &n) {
    const std::string rhs = eval(*n.rhs);
    *this << "(" << ctxt.bnot(n) << " " << rhs << ")";
  }

  void visit_bor(const Bor &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(" << ctxt.bor(n) << " " << lhs << " " << rhs << ")";
  }

  void visit_element(const Element &n) {
    const std::string array = eval(*n.array);
    const std::string index = eval(*n.index);
    *this << "(select " << array << " " << index << ")";
  }

  void visit_exprid(const ExprID &n) {
    *this << ctxt.lookup_symbol(n.value->unique_id, n);
  }

  void visit_eq(const Eq &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(= " << lhs << " " << rhs << ")";
  }

  void visit_div(const Div &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(" << ctxt.div(n) << " " << lhs << " " << rhs << ")";
  }

  void visit_geq(const Geq &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(" << ctxt.geq(n) << " " << lhs << " " << rhs << ")";
  }

  void visit_gt(const Gt &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(" << ctxt.gt(n) << " " << lhs << " " << rhs << ")";
  }

  void visit_implication(const Implication &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(=> " << lhs << " " << rhs << ")";
  }

  void visit_isundefined(const IsUndefined &n) {
    throw Error("SMT translation of isundefined expressions is not supported",
      n.loc);
  }

  void visit_leq(const Leq &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(" << ctxt.leq(n) << " " << lhs << " " << rhs << ")";
  }

  void visit_lsh(const Lsh &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(" << ctxt.lsh(n) << " " << lhs << " " << rhs << ")";
  }

  void visit_lt(const Lt &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(" << ctxt.lt(n) << " " << lhs << " " << rhs << ")";
  }

  void visit_mod(const Mod &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(" << ctxt.mod(n) << " " << lhs << " " << rhs << ")";
  }

  // I cannot imagine any use case for doing a one-shot translation of an entire
  // model to SMT, but see no reason to prevent the user doing this if they come
  // up with a need for it
  void visit_model(const Model &n) {
    for (const Ptr<Node> &c : n.children) {
      // use eval() here because we do not care about any returned constructions
      // and want to discard them
      (void)eval(*c);
    }
  }

  void visit_mul(const Mul &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(" << ctxt.mul(n) << " " << lhs << " " << rhs << ")";
  }

  void visit_negative(const Negative &n) {
    const std::string rhs = eval(*n.rhs);
    *this << "(" << ctxt.neg(n) << " " << rhs << ")";
  }

  void visit_neq(const Neq &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(not (= " << lhs << " " << rhs << "))";
  }

  void visit_number(const Number &n) {
    *this << ctxt.numeric_literal(n.value, n);
  }

  void visit_not(const Not &n) {
    const std::string rhs = eval(*n.rhs);
    *this << "(not " << rhs << ")";
  }

  void visit_or(const Or &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(or " << lhs << " " << rhs << ")";
  }

  void visit_range(const Range &n) {

    // construct the bounds of this range
    const std::string min = eval(*n.min);
    const std::string max = eval(*n.max);

    // define this type
    const std::string tid = ctxt.make_type("Int");

    // generate a name to use for the predicate for a value in this type
    const std::string guard = get_type_guard(tid);

    ctxt
      << "(define-fun " << guard << " ((x!1 " << tid << ")) Bool (and "
        << "(" << ctxt.geq(n) << " x!1 " << min << ") "
        << "(" << ctxt.leq(n) << " x!1 " << max << ")))\n";

    // pass the name we used back any caller
    *this << tid;
  }

  void visit_rsh(const Rsh &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(" << ctxt.rsh(n) << " " << lhs << " " << rhs << ")";
  }

  void visit_sub(const Sub &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(" << ctxt.sub(n) << " " << lhs << " " << rhs << ")";
  }

  void visit_ternary(const Ternary &n) {
    const std::string cond = eval(*n.cond);
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(ite " << cond << " " << lhs << " " << rhs << ")";
  }

  void visit_typedecl(const TypeDecl &n) {

    // ensure this type is defined
    const std::string type = eval(*n.value);

    // define this type
    const std::string name = ctxt.make_type(n.unique_id, type);

    // define a type guard corresponding to the inner type’s guard
    const std::string inner_guard = get_type_guard(type);
    const std::string guard = get_type_guard(name);
    ctxt << "(define-fun " << guard << " ((x!1 " << name << ")) Bool ("
      << inner_guard << " x!1))\n";

    // pass the name we used back to any caller
    *this << name;
  }

  void visit_vardecl(const VarDecl &n) {

    // TODO: the below logic won't work for VarDecls that represent record
    // fields

    // mint a new name for this declaration
    const std::string name = ctxt.register_symbol(n.unique_id);

    // establish the type of this variable
    const std::string type = eval(*n.type);
    const std::string guard = get_type_guard(type);

    // declare the variable and its constraints
    ctxt << "(declare-fun " << name << " () " << type << ")\n"
         << "(assert (" << guard << " " << name << "))\n";

    // pass the name we used back to any caller
    *this << name;
  }

  void visit_xor(const Xor &n) {
    const std::string lhs = eval(*n.lhs);
    const std::string rhs = eval(*n.rhs);
    *this << "(" << ctxt.bxor(n) << " " << lhs << " " << rhs << ")";
  }

 private:
  Translator &operator<<(const std::string &s) {
    ret << s;
    return *this;
  }

  // encapsulate visiting a Node and extracting a returned string
  std::string eval(const Node &n) {

    // the ret buffer is not expected to be used across multiple visitation
    // calls
    assert(ret.str() == "");

    // visit the node
    dispatch(n);

    // extract the result of this visitation
    const std::string r = ret.str();
    ret.str("");

    return r;
  }

}; }

void to_smt(std::ostream &out, const Node &n, SMTContext &ctxt) {
  out << to_smt(n, ctxt);
}

std::string to_smt(const Node &n, SMTContext &ctxt) {
  Translator t(ctxt);
  t.dispatch(n);
  return t.str();
}

}
