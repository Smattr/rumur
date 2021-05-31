#include "codegen.h"
#include "../../common/isa.h"
#include "module_name.h"
#include <cstddef>
#include <cassert>
#include <iostream>
#include <rumur/rumur.h>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace rumur;

// is the given parameter, representing a for step, known to be 1?
static bool is_one_step(const Ptr<Expr> &step) {
  if (step == nullptr)
    return true;
  if (!step->constant())
    return false;
  return step->constant_fold() == 1;
}

namespace {

/** a visitor that prints Uclid5 code
 *
 * While murphi2uclid only attempts to translate Models to Uclid5, this visitor
 * is actually capable of starting translation at a Node of any type.
 */
class Printer : public ConstBaseTraversal {

private:
  std::ostream &o; ///< output stream to emit code to

  size_t indentation = 0; ///< current indentation level

  std::vector<std::string> vars; ///< state variables we have seen

  size_t id = 0; ///< counter used for constructing unique symbols

  const std::vector<Comment> &comments;
  ///< comments from the original source file

  std::vector<bool> emitted;
  ///< whether each comment has been written to the output yet

  std::vector<const Quantifier*> params;
  ///< parameters in the ruleset context we are currently within

public:
  Printer(std::ostream &o_, const std::vector<Comment> &comments_)
      : o(o_), comments(comments_), emitted(comments_.size(), false) {}

  void visit_add(const Add &n) final {
    *this << "(" << *n.lhs << " + " << *n.rhs << ")";
  }

  void visit_aliasdecl(const AliasDecl &) final {
    throw std::logic_error("alias declaration should have been rejected during "
                           "check()");
  }

  void visit_aliasrule(const AliasRule &) final {
    throw std::logic_error("alias rule should have been rejected during "
                           "check()");
  }

  void visit_aliasstmt(const AliasStmt &) final {
    throw std::logic_error("alias statement should have been rejected during "
                           "check()");
  }

  void visit_and(const And &n) final {
    *this << "(" << *n.lhs << " && " << *n.rhs << ")";
  }

  void visit_array(const Array &n) final {
    // TODO: multi-dimensional arrays
    *this << "[" << *n.index_type << "]" << *n.element_type;
  }

  void visit_assignment(const Assignment &n) final {
    // assume we are within a procedure or init and so can use synchronous
    // assignment

    // special case function call assignment that has its own syntax
    if (isa<FunctionCall>(n.rhs)) {
      *this << tab() << "call (" << *n.lhs <<  ") = " << *n.rhs << ";\n";
      return;
    }

    *this << tab() << *n.lhs << " = " << *n.rhs << ";\n";
  }

  void visit_band(const Band &n) final {
    *this << "(" << *n.lhs << " & " << *n.rhs << ")";
  }

  void visit_bnot(const Bnot &n) final {
    *this << "(~" << *n.rhs << ")";
  }

  void visit_bor(const Bor &n) final {
    *this << "(" << *n.lhs << " | " << *n.rhs << ")";
  }

  void visit_clear(const Clear &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_constdecl(const ConstDecl &n) final {

    // emit as a symbolic constant
    *this << tab() << "const " << n.name << " : " << *n.get_type() << ";\n";

    // constrain it to have exactly its known value
    *this << tab() << "assume " << n.name << "_value: " << n.name << " == "
      << *n.value << ";\n";
  }

  void visit_div(const Div &) final {
    throw std::logic_error("/ should have been rejected during check()");
  }

  void visit_element(const Element &n) final {
    *this << *n.array << "[" << *n.index << "]";
  }

  void visit_enum(const Enum &n) final {
    *this << "enum {";
    indent();

    std::string sep;
    for (const std::pair<std::string, location> &m : n.members) {
      *this << sep << "\n" << tab() << m.first;
      sep = ",";
    }
    *this << "\n";

    dedent();
    *this << tab() << "}";
  }

  void visit_eq(const Eq &n) final {
    *this << "(" << *n.lhs << " == " << *n.rhs << ")";
  }

  void visit_errorstmt(const ErrorStmt &n) final {
    // no direct equivalent of this, so just emit the message as a comment and
    // then an always-failing assertion
    *this << tab() << "// " << n.message << "\n"
          << tab() << "assert (false);\n";
  }

  void visit_exists(const Exists &n) final {

    if (n.quantifier.type != nullptr) {
      *this << "(exists (" << n.quantifier.name << " : " << *n.quantifier.type
        << ") :: " << *n.expr << ")";
      return;
    }

    if (is_one_step(n.quantifier.step)) {
      *this << "(exists (" << n.quantifier.name << " : integer) :: ("
        << "(" << n.quantifier.name << " >= " << *n.quantifier.from << ") && "
        << "(" << n.quantifier.name << " <= " << *n.quantifier.to << ") && "
        << *n.expr << "))";
        return;
    }

    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_exprid(const ExprID &n) final {
    *this << n.id;
  }

  void visit_field(const Field &n) final {
    *this << *n.record << "." << n.field;
  }

  void visit_for(const For &n) final {

    // do we need to generate a while loop instead of a for loop
    bool needs_while = !is_one_step(n.quantifier.step);

    // non-1 steps need to be handled as a while loop
    if (needs_while) {
      *this << tab() << "{\n";
      indent();

      const std::string lb = make_symbol("lower");
      *this << tab() << "var " << lb << " : integer;\n"
            << tab() << lb << " = " << *n.quantifier.from << ";\n";

      const std::string ub = make_symbol("upper");
      *this << tab() << "var " << ub << " : integer;\n"
            << tab() << ub << " = " << *n.quantifier.to << ";\n";

      const std::string &i = n.quantifier.name;
      assert(n.quantifier.step != nullptr);
      const Expr &step = *n.quantifier.step;
      *this << tab() << "var " << i << " : integer;\n"
            << tab() << i << " = " << lb << ";\n"
            << tab() << "while ((" << lb << " <= " << ub << " && " << i
              << " <= " << ub << ") ||\n"
            << tab() << "       (" << lb << " > " << ub << " && " << i << " >= "
              << ub << ")) {\n";
      indent();

      for (const Ptr<Stmt> &s : n.body) {
        emit_leading_comments(*s);
        *this << *s;
      }

      *this << tab() << i << " = " << i << " + " << step << ";\n";

      dedent();
      *this << tab() << "}\n";
      dedent();
      *this << tab() << "}\n";
      return;
    }

    *this << tab() << "for " << n.quantifier << " {\n";
    indent();

    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }

    dedent();
    *this << tab() << "}\n";
  }

  void visit_forall(const Forall &n) final {

    if (n.quantifier.type != nullptr) {
      *this << "(forall (" << n.quantifier.name << " : " << *n.quantifier.type
        << ") :: " << *n.expr << ")";
      return;
    }

    if (is_one_step(n.quantifier.step)) {
      *this << "(forall (" << n.quantifier.name << " : integer) :: ("
        << "(" << n.quantifier.name << " >= " << *n.quantifier.from << ") && "
        << "(" << n.quantifier.name << " <= " << *n.quantifier.to << ") && "
        << *n.expr << "))";
        return;
    }

    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_function(const Function &n) final {

    // TODO
    for (const Ptr<VarDecl> &p : n.parameters) {
      if (!p->is_readonly()) {
        throw Error("unsupported Murphi node", p->loc);
      }
    }

    *this << tab() << "procedure " << n.name << "(";
    {
      std::string sep;
      for (const Ptr<VarDecl> &p : n.parameters) {
        *this << sep << p->name << ": " << *p->get_type();
        sep = ", ";
      }
    }
    *this << ")\n";
    indent();

    if (n.return_type != nullptr)
      *this << tab() << "returns (__return: " << *n.return_type << ")\n";

    // conservatively allow the function to modify anything, to avoid having to
    // inspect its body
    if (!vars.empty()) {
      *this << tab() << "modifies ";
      std::string sep = "";
      for (const std::string &v : vars) {
        *this << sep << v;
        sep = ", ";
      }
      *this << ";\n";
    }

    dedent();
    *this << tab() << "{\n";
    indent();

    for (const Ptr<Decl> &d : n.decls) {
      emit_leading_comments(*d);
      *this << *d;
    }

    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }

    dedent();
    *this << tab() << "}\n";
  }

  void visit_functioncall(const FunctionCall &n) final {
    *this << n.name << "(";

    std::string sep;
    for (const Ptr<Expr> &a : n.arguments) {
      *this << sep << *a;
      sep = ", ";
    }

    *this << ")";
  }

  void visit_geq(const Geq &n) final {
    *this << "(" << *n.lhs << " >= " << *n.rhs << ")";
  }

  void visit_gt(const Gt &n) final {
    *this << "(" << *n.lhs << " > " << *n.rhs << ")";
  }

  void visit_if(const If &n) final {
    assert(!n.clauses.empty() && "if statement with no content");
    bool first = true;
    for (const IfClause &c : n.clauses) {
      if (first) {
        *this << tab() << c;
        first = false;
      } else {
        *this << " else {\n";
        indent();
        *this << tab() << c;
      }
    }
    *this << "\n";
  }

  void visit_ifclause(const IfClause &n) final {
    if (n.condition != nullptr) {
      bool needs_brackets = !isa<BinaryExpr>(n.condition);
      *this << "if ";
      if (needs_brackets)
        *this << "(";
      *this << *n.condition;
      if (needs_brackets)
        *this << ")";
      *this << " {\n";
      indent();
    }
    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }
    dedent();
    *this << tab() << "}";
  }

  void visit_implication(const Implication &n) final {
    *this << "(" << *n.lhs << " ==> " << *n.rhs << ")";
  }

  void visit_isundefined(const IsUndefined &) final {
    throw std::logic_error("isundefined should have been rejected during "
                           "check()");
  }

  void visit_leq(const Leq &n) final {
    *this << "(" << *n.lhs << " <= " << *n.rhs << ")";
  }

  void visit_lsh(const Lsh &) final {
    throw std::logic_error("<< should have been rejected during check()");
  }

  void visit_lt(const Lt &n) final {
    *this << "(" << *n.lhs << " < " << *n.rhs << ")";
  }

  void visit_mod(const Mod &) final {
    throw std::logic_error("% should have been rejected during check()");
  }

  void visit_model(const Model &n) final {
    emit_leading_comments(n);

    // output module header
    *this << "module " << module_name << " {\n";
    indent();

    for (const Ptr<Node> &c : n.children) {
      emit_leading_comments(*c);
      *this << *c;
    }

    // close module
    dedent();
    *this << "}\n";
  }

  void visit_mul(const Mul &n) final {
    *this << "(" << *n.lhs << " * " << *n.rhs << ")";
  }

  void visit_negative(const Negative &n) final {
    *this << "-" << *n.rhs;
  }

  void visit_neq(const Neq &n) final {
    *this << "(" << *n.lhs << " != " << *n.rhs << ")";
  }

  void visit_not(const Not &n) final {
    *this << "!" << *n.rhs;
  }

  void visit_number(const Number &n) final {
    *this << n.value.get_str();
  }

  void visit_or(const Or &n) final {
    *this << "(" << *n.lhs << " || " << *n.rhs << ")";
  }

  void visit_procedurecall(const ProcedureCall &n) final {
    *this << tab() << "call " << n.call << ";\n";
  }

  void visit_property(const Property &) final {
    throw std::logic_error("property should have been handled in its parent ("
      "either PropertyRule or PropertyStmt)");
  }

  void visit_propertyrule(const PropertyRule &n) final {

    *this << "\n" << tab();
    switch (n.property.category) {

    case Property::ASSERTION:
      *this << "invariant";
      break;

    case Property::ASSUMPTION:
      *this << "assume";
      break;

    case Property::COVER:
      throw std::logic_error("cover property should have been rejected during "
                             "check()");

    case Property::LIVENESS:
      *this << "property[LTL] ";
      break;

    }
    *this << " " << n.name << ": ";

    for (const Quantifier *q : params) {
      *this << "(forall (" << q->name << " : ";
      if (q->type == nullptr) {
        *this << "integer";
      } else {
        *this << *q->type;
      }
      *this << ") (";
      if (q->type == nullptr) {
        if (!is_one_step(q->step)) // TODO
          throw Error("unsupported Murphi node", n.loc);
        *this << q->name << " < " << *q->from << " || "
          << q->name << " > " << *q->to << " || ";
      }
    }

    if (n.property.category == Property::LIVENESS)
      *this << "G(F(";

    *this << *n.property.expr;

    if (n.property.category == Property::LIVENESS)
      *this << "))";

    for (size_t i = 0; i < params.size(); ++i)
      *this << "))";

    *this << ";\n";
  }

  void visit_propertystmt(const PropertyStmt &n) final {

    // there is no equivalent of property messages, so just emit this as a
    // comment
    if (n.message != "") {
      *this << tab() << "// ";
      for (const char &c : n.message) {
        *this << c;
        if (c == '\n')
          *this << tab() << "// ";
      }
      *this << "\n";
    }

    switch (n.property.category) {
    case Property::ASSERTION:
      *this << tab() << "assert " << *n.property.expr << ";\n";
      break;
    case Property::ASSUMPTION:
      *this << tab() << "assume " << *n.property.expr << ";\n";
      break;
    case Property::COVER:
      throw std::logic_error("cover statement should have been rejected during "
                             "check()");
    case Property::LIVENESS:
      throw std::logic_error("liveness statement should have been rejected "
                             "during check()");
    }
  }

  void visit_put(const Put &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_quantifier(const Quantifier &n) final {
    assert(is_one_step(n.step) && "non-trivial step in quantifier visitation");

    if (n.type == nullptr) {
      *this << n.name << " in range(" << *n.from << ", " << *n.to << ")";

    } else {
      *this << "(" << n.name << " : " << *n.type << ") in range(";

      const Ptr<TypeExpr> resolved = n.type->resolve();

      if (auto r = dynamic_cast<const Range*>(resolved.get())) {
        *this << *r->min << ", " << *r->max;

      } else if (auto s = dynamic_cast<const Scalarset*>(resolved.get())) {
        *this << "0, " << *s->bound << " - 1";

      } else if (auto e = dynamic_cast<const Enum*>(resolved.get())) {
        if (e->members.empty())
          throw Error("you cannot iterate over an enum with no members", n.loc);
        *this << e->members[0].first << ", " << e->members.back().first;

      } else {
        assert(!"unhandled simple type");
      }

      *this << ")";
    }
  }

  void visit_range(const Range&) final {
    *this << "integer";

    // TODO: range limits
  }

  void visit_record(const Record &n) final {
    *this << "record {\n";
    indent();

    std::string sep;
    for (const Ptr<VarDecl> &f : n.fields) {
      *this << sep << tab() << f->name << " : " << *f->get_type();
      sep = ",\n";
    }

    *this << "\n";
    dedent();
    *this << tab() << "}";
  }

  void visit_return(const Return &n) final {
    // only relevant if an actual value is being returned
    if (n.expr != nullptr)
      *this << tab() << "__return = " << *n.expr << ";\n";
  }

  void visit_rsh(const Rsh &) final {
    throw std::logic_error(">> should have been rejected during check()");
  }

  void visit_ruleset(const Ruleset &n) final {

    // make our parameters visible to children as we descend
    for (const Quantifier &q : n.quantifiers)
      params.push_back(&q);

    for (const Ptr<Rule> &r : n.rules)
      *this << *r;

    assert(params.size() >= n.quantifiers.size() &&
      "ruleset parameter management out of sync");
    for (size_t i = 0; i < n.quantifiers.size(); ++i) {
      size_t j [[gnu::unused]] = params.size() - n.quantifiers.size() + i;
      assert(params[j] == &n.quantifiers[i] &&
        "ruleset parameter management out of sync");
    }

    // remove the parameters as we ascend
    params.resize(params.size() - n.quantifiers.size());
  }

  void visit_scalarset(const Scalarset&) final {
    *this << "integer";

    // TODO: range limits
  }

  void visit_simplerule(const SimpleRule &n) final {

    if (n.guard != nullptr) {
      *this << "\n" << tab() << "define guard_" << n.name << "(";
      emit_params();
      *this << ") : boolean = " << *n.guard << ";\n";
    }

    // emit rules as procedures, so we can use synchronous assignment
    *this << "\n" << tab() << "procedure rule_" << n.name << "(";
    emit_params();
    *this << ")\n";

    // conservatively allow the rule to modify anything, to avoid having to
    // inspect its body
    if (!vars.empty()) {
      indent();
      *this << tab() << "modifies ";
      std::string sep = "";
      for (const std::string &v : vars) {
        *this << sep << v;
        sep = ", ";
      }
      *this << ";\n";
      dedent();
    }

    *this << tab() << "{\n";
    indent();

    for (const Ptr<Decl> &d : n.decls) {
      emit_leading_comments(*d);
      *this << *d;
    }

    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }

    dedent();
    *this << tab() << "}\n";
  }

  void visit_startstate(const StartState &n) final {

    *this << "\n" << tab() << "procedure startstate_" << n.name << "(";
    emit_params();
    *this << ")\n";

    // conservatively allow the startstate to modify anything, to avoid having
    // to inspect its body
    if (!vars.empty()) {
      indent();
      *this << tab() << "modifies ";
      std::string sep = "";
      for (const std::string &v : vars) {
        *this << sep << v;
        sep = ", ";
      }
      *this << ";\n";
      dedent();
    }

    *this << tab() << "{\n";
    indent();

    for (const Ptr<Decl> &d : n.decls) {
      emit_leading_comments(*d);
      *this << *d;
    }

    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }

    dedent();
    *this << tab() << "}\n";
  }

  void visit_sub(const Sub &n) final {
    *this << "(" << *n.lhs << " - " << *n.rhs << ")";
  }

  void visit_switch(const Switch &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_switchcase(const SwitchCase &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_ternary(const Ternary &n) final {
    bool needs_brackets = !isa<BinaryExpr>(n.cond);
    *this << "(if ";
    if (needs_brackets)
      *this << "(";
    *this << *n.cond;
    if (needs_brackets)
      *this << ")";
    *this << " then " << *n.lhs << " else " << *n.rhs << ")";
  }

  void visit_typedecl(const TypeDecl &n) final {
    *this << tab() << "type " << n.name << " = " << *n.value << ";\n";
  }

  void visit_typeexprid(const TypeExprID &n) final {
    // no need for special “boolean” handling because it has the same
    // spelling in Murphi and Uclid5
    *this << n.name;
  }

  void visit_undefine(const Undefine &n) final {
    // Uclid5 has no direct equivalent of the Murphi `undefine` statement.
    // However, it is hoped that havocking the variable has a similar effect.
    // Namely, encouraging the model checker to treat any following read without
    // an intervening write as an error. Obviously this is not exactly what
    // happens. See the murphi2uclid man page for some further discussion.
    *this << tab() << "havoc " << *n.rhs << ";\n";
  }

  void visit_vardecl(const VarDecl &n) final {
    *this << tab() << "var " << n.name << " : " << *n.get_type() << ";\n";

    vars.push_back(n.name);
  }

  void visit_while(const While &n) final {

    bool needs_brackets = !isa<BinaryExpr>(n.condition);
    *this << tab() << "while ";
    if (needs_brackets)
      *this << "(";
    *this << *n.condition;
    if (needs_brackets)
      *this << ")";
    *this << " {\n";
    indent();

    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }

    dedent();
    *this << tab() << "}\n";
  }

  void visit_xor(const Xor &n) final {
    *this << "(" << *n.lhs << " ^ " << *n.rhs << ")";
  }

private:
  // wrappers to allow more readable code above
  Printer &operator<<(const std::string &s) {
    o << s;
    return *this;
  }
  Printer &operator<<(char c) {
    o << c;
    return *this;
  }
  Printer &operator<<(const Node &n) {
    n.visit(*this);
    return *this;
  }

  void indent() { ++indentation; }

  void dedent() {
    assert(indentation > 0);
    --indentation;
  }

  std::string tab() { return std::string(indentation * 2, ' '); }

  /// create a unique symbol for use in generated code
  std::string make_symbol(const std::string &name) {
    // FIXME: better strategy for avoiding collisions with user symbols
    return "__sym_" + name + std::to_string(id++);
  }

  /// print the current list of enclosing parameters, assuming we are within
  /// something like a procedure’s parameter list
  void emit_params(void) {
    std::string sep;
    for (const Quantifier *q : params) {
      *this << sep << q->name << " : ";
      if (q->type == nullptr) {
        *this << "integer";
      } else {
        *this << *q->type;
      }
      sep = ", ";
    }
  }

  /// print any source comments to prior to the given node that have not yet
  /// been printed
  size_t emit_leading_comments(const Node &n) {
    size_t count = 0;
    size_t i = 0;
    for (const Comment &c : comments) {
      // has this not yet been printed?
      if (!emitted[i]) {
        // does this precede the given node?
        if (c.loc.end.line < n.loc.begin.line ||
            (c.loc.end.line == n.loc.begin.line &&
             c.loc.end.column <= n.loc.begin.column)) {

          // do some white space adjustment for multiline comments
          if (c.multiline) {
            o << tab() << "/*";
            bool dropping = false;
            for (const char &b : c.content) {
              if (b == '\n') {
                o << "\n" << tab() << " ";
                dropping = true;
              } else if (dropping) {
                if (!isspace(b)) {
                  o << b;
                  dropping = false;
                }
              } else {
                o << b;
              }
            }
            o << "*/\n";

          } else { // single line comments can be emitted simpler
            o << tab() << "//" << c.content << "\n";
          }

          emitted[i] = true;
        }
      }
      ++i;
    }
    return count;
  }
};

} // namespace

void codegen(const Node &n, const std::vector<Comment> &comments,
             std::ostream &out) {
  Printer p(out, comments);
  n.visit(p);
}
