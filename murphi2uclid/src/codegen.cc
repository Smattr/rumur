#include "codegen.h"
#include "../../common/isa.h"
#include "module_name.h"
#include <cstddef>
#include <cassert>
#include <iostream>
#include <rumur/rumur.h>
#include <stdexcept>
#include <string>
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

public:
  Printer(std::ostream &o_, const std::vector<Comment> &comments_)
      : o(o_), comments(comments_), emitted(comments_.size(), false) {}

  void visit_add(const Add &n) final {
    *this << "(" << *n.lhs << " + " << *n.rhs << ")";
  }

  void visit_aliasdecl(const AliasDecl &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_aliasrule(const AliasRule &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_aliasstmt(const AliasStmt &n) final {
    throw Error("unsupported Murphi node", n.loc);
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

  void visit_div(const Div &n) final {
    *this << "(" << *n.lhs << " / " << *n.rhs << ")";
  }

  void visit_element(const Element &n) final {
    *this << *n.array << "[" << *n.index << "]";
  }

  void visit_enum(const Enum &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_eq(const Eq &n) final {
    *this << "(" << *n.lhs << " == " << *n.rhs << ")";
  }

  void visit_errorstmt(const ErrorStmt &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_exists(const Exists &n) final {
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
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_function(const Function &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_functioncall(const FunctionCall &n) final {
    throw Error("unsupported Murphi node", n.loc);
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

  void visit_isundefined(const IsUndefined &n) final {
    throw Error("unsupported Murphi node", n.loc);
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
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_property(const Property &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_propertyrule(const PropertyRule &n) final {
    throw Error("unsupported Murphi node", n.loc);
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
    default:
      throw Error("unsupported Murphi node", n.loc);
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
      *this << "(" << n.name << " : " << *n.type << ")";
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
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_rsh(const Rsh &) final {
    throw std::logic_error(">> should have been rejected during check()");
  }

  void visit_ruleset(const Ruleset &n) final {
    throw Error("unsupported Murphi node", n.loc);
  }

  void visit_scalarset(const Scalarset&) final {
    *this << "integer";

    // TODO: range limits
  }

  void visit_simplerule(const SimpleRule &n) final {

    if (n.guard != nullptr) {
      *this << "\n" << tab() << "define guard_" << n.name << "() : boolean = "
        << *n.guard << ";\n";
    }

    // emit rules as procedures, so we can use synchronous assignment
    *this << "\n" << tab() << "procedure rule_" << n.name << "()\n";

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
    // TODO: how to deal with models that have more than one startstate?
    *this << "\n" << tab() << "init {\n";
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
    throw Error("unsupported Murphi node", n.loc);
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
    dispatch(n);
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
  p.dispatch(n);
}
