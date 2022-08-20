#include "codegen.h"
#include "../../common/isa.h"
#include "is_one_step.h"
#include "options.h"
#include <cassert>
#include <cstddef>
#include <iostream>
#include <rumur/rumur.h>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace rumur;

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

  std::vector<const Quantifier *> params;
  ///< parameters in the ruleset context we are currently within

  std::vector<std::string> to_return;
  ///< function parameters that are 'var' (not read-only) and hence we need to
  ///< track and then deal with in return statements

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
    if (auto c = dynamic_cast<const FunctionCall *>(n.rhs.get())) {
      *this << tab() << "call (" << *n.lhs;

      // We need to also assign to any 'var' parameters. The function itself
      // will have been translate to something that yields these as extra return
      // values.
      assert(
          c->arguments.size() == c->function->parameters.size() &&
          "function call with a different number of arguments than its target");
      for (size_t i = 0; i < c->arguments.size(); ++i) {
        if (!c->function->parameters[i]->is_readonly()) {
          // FIXME: not strictly safe to emit this and then emit it again in the
          // call itself; this is not guaranteed to be a pure expression
          *this << ", " << *c->arguments[i];
        }
      }

      *this << ") = " << *c << ";\n";
      return;
    }

    *this << tab() << *n.lhs << " = " << *n.rhs << ";\n";
  }

  void visit_band(const Band &n) final {
    *this << "(" << *n.lhs << " & " << *n.rhs << ")";
  }

  void visit_bnot(const Bnot &n) final { *this << "(~" << *n.rhs << ")"; }

  void visit_bor(const Bor &n) final {
    *this << "(" << *n.lhs << " | " << *n.rhs << ")";
  }

  void visit_clear(const Clear &n) final {

    const Ptr<TypeExpr> type = n.rhs->type()->resolve();

    if (auto r = dynamic_cast<const Range *>(type.get())) {
      *this << tab() << *n.rhs << " = " << r->min->constant_fold().get_str()
            << ";\n";
      return;
    }

    if (isa<Scalarset>(type)) {
      *this << tab() << *n.rhs << " = 0;\n";
      return;
    }

    if (auto e = dynamic_cast<const Enum *>(type.get())) {
      assert(!e->members.empty() && "enum with no members");
      *this << tab() << *n.rhs << " = " << e->members[0].first << "\n;";
      return;
    }

    throw std::logic_error("clear of complex type should have been rejected "
                           "during check()");
  }

  void visit_constdecl(const ConstDecl &n) final {

    // emit as a symbolic constant
    *this << tab() << "const " << n.name << " : ";

    const Ptr<TypeExpr> type = n.get_type();
    if (type->is_boolean()) {
      *this << "boolean";
    } else {
      *this << *type;
    }
    *this << ";\n";

    // constrain it to have exactly its known value
    *this << tab() << "assume " << n.name << "_value: " << n.name
          << " == " << *n.value << ";\n";
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
      *this << "(exists (" << n.quantifier.name << " : " << numeric_type
            << ") :: (" << n.quantifier.name << " >= " << *n.quantifier.from
            << " && " << n.quantifier.name << " <= " << *n.quantifier.to
            << " && " << *n.expr << "))";
      return;
    }

    throw std::logic_error("exists should have been rejected during check()");
  }

  void visit_exprid(const ExprID &n) final { *this << n.id; }

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
      *this << tab() << "var " << lb << " : " << numeric_type << ";\n";

      const std::string ub = make_symbol("upper");
      *this << tab() << "var " << ub << " : " << numeric_type << ";\n";

      const std::string &i = n.quantifier.name;
      assert(n.quantifier.step != nullptr);
      const Expr &step = *n.quantifier.step;
      *this << tab() << "var " << i << " : " << numeric_type << ";\n";

      *this << tab() << lb << " = " << *n.quantifier.from << ";\n"
            << tab() << ub << " = " << *n.quantifier.to << ";\n"
            << tab() << i << " = " << lb << ";\n"
            << tab() << "while ((" << lb << " <= " << ub << " && " << i
            << " <= " << ub << ") ||\n"
            << tab() << "       (" << lb << " > " << ub << " && " << i
            << " >= " << ub << ")) {\n";
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
      *this << "(forall (" << n.quantifier.name << " : " << numeric_type
            << ") :: (" << n.quantifier.name << " < " << *n.quantifier.from
            << " && " << n.quantifier.name << " > " << *n.quantifier.to
            << " && " << *n.expr << "))";
      return;
    }

    throw std::logic_error("forall should have been rejected during check()");
  }

  void visit_function(const Function &n) final {

    *this << "\n" << tab() << "procedure " << n.name << "(";
    {
      std::string sep;
      for (const Ptr<VarDecl> &p : n.parameters) {
        *this << sep << p->name << ": " << *p->get_type();
        sep = ", ";
      }
    }
    *this << ")\n";
    indent();

    {
      std::string sep = tab() + "returns (";
      std::string ender;
      if (n.return_type != nullptr) {
        *this << sep << "__return: " << *n.return_type;
        sep = ", ";
        ender = ")\n";
      }
      for (const Ptr<VarDecl> &p : n.parameters) {
        if (!p->is_readonly()) {
          *this << sep << "__" << p->name << ": " << *p->get_type();
          sep = ", ";
          ender = ")\n";
          to_return.push_back(p->name);
        }
      }
      *this << ender;
    }

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

    // if we reached the end of the function, we may have seen no return
    // statement but need to propagate parameters passed in by value to return
    // values
    for (const std::string &s : to_return)
      *this << tab() << "__" << s << " = " << s << ";\n";

    dedent();
    *this << tab() << "}\n";

    to_return.clear();
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
        *this << c;
        first = false;
      } else {
        *this << " else {\n";
        indent();
        *this << c;
      }
    }
    first = true;
    for (const IfClause &c : n.clauses) {
      if (first) {
        first = false;
      } else if (c.condition != nullptr) { // did we indent in visit_ifclause?
        dedent();
        *this << "\n" << tab() << "}";
      }
    }
    *this << "\n";
  }

  void visit_ifclause(const IfClause &n) final {
    if (n.condition != nullptr) {
      bool needs_brackets = !isa<BinaryExpr>(n.condition);
      *this << tab() << "if ";
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

    // force more natural placement for file-leading comments
    if (!n.children.empty())
      emit_leading_comments(*n.children[0]);

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

  void visit_negative(const Negative &n) final { *this << "-" << *n.rhs; }

  void visit_neq(const Neq &n) final {
    *this << "(" << *n.lhs << " != " << *n.rhs << ")";
  }

  void visit_not(const Not &n) final { *this << "!" << *n.rhs; }

  void visit_number(const Number &n) final {
    *this << n.value.get_str();

    // if we are using a bit-vector type, we need to suffix numeric literals
    // with it
    if (numeric_type != "integer")
      *this << numeric_type;
  }

  void visit_or(const Or &n) final {
    *this << "(" << *n.lhs << " || " << *n.rhs << ")";
  }

  void visit_procedurecall(const ProcedureCall &n) final {

    // Murphi permits calling a function that return a value and then
    // discarding the result. However, this is an error in Uclid5. So if we have
    // such a situation, work around this with an ignored local variable.
    const Ptr<TypeExpr> &ret = n.call.function->return_type;
    if (ret != nullptr) {

      // open a scope so we can declare a new local
      *this << tab() << "{\n";
      indent();

      const std::string s = make_symbol("ignored");
      *this << tab() << "var " << s << " : " << *ret << ";\n";

      *this << tab() << "call (" << s;

      // also deal with any 'var' parameters
      assert(
          n.call.arguments.size() == n.call.function->parameters.size() &&
          "function call with a different number of arguments than its target");
      for (size_t i = 0; i < n.call.arguments.size(); ++i) {
        if (!n.call.function->parameters[i]->is_readonly()) {
          // FIXME: not strictly safe to emit this and then emit it again in the
          // call itself; this is not guaranteed to be a pure expression
          *this << ", " << *n.call.arguments[i];
        }
      }

      *this << ") = " << n.call << ";\n";

      dedent();
      *this << tab() << "}\n";

      return;
    }

    *this << tab() << "call ";

    // handle any 'var' parameters this function may have
    bool has_var = false;
    assert(
        n.call.arguments.size() == n.call.function->parameters.size() &&
        "function call with a different number of arguments than its target");
    std::string sep = "(";
    for (size_t i = 0; i < n.call.arguments.size(); ++i) {
      if (!n.call.function->parameters[i]->is_readonly()) {
        // FIXME: not strictly safe to emit this and then emit it again in the
        // call itself; this is not guaranteed to be a pure expression
        *this << sep << *n.call.arguments[i];
        sep = ", ";
        has_var = true;
      }
    }
    if (has_var)
      *this << ") = ";

    *this << n.call << ";\n";
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
        *this << numeric_type;
      } else {
        *this << *q->type;
      }
      *this << ") :: (";
      if (q->type == nullptr) {
        if (!is_one_step(q->step)) // TODO
          throw std::logic_error("property should have been rejected during "
                                 "check()");
        *this << q->name << " < " << *q->from << " || " << q->name << " > "
              << *q->to << " || ";
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

  void visit_put(const Put &) final {
    throw std::logic_error("put statement should have been rejected during "
                           "check()");
  }

  void visit_quantifier(const Quantifier &n) final {
    assert(is_one_step(n.step) && "non-trivial step in quantifier visitation");

    if (n.type == nullptr) {
      *this << n.name << " in range(" << *n.from << ", " << *n.to << ")";

    } else {
      *this << "(" << n.name << " : " << *n.type << ") in range(";

      const Ptr<TypeExpr> resolved = n.type->resolve();

      if (auto r = dynamic_cast<const Range *>(resolved.get())) {
        *this << *r->min << ", " << *r->max;

      } else if (auto s = dynamic_cast<const Scalarset *>(resolved.get())) {
        *this << "0, " << *s->bound << " - 1";

      } else if (auto e = dynamic_cast<const Enum *>(resolved.get())) {
        if (e->members.empty())
          throw Error("you cannot iterate over an enum with no members", n.loc);
        *this << e->members[0].first << ", " << e->members.back().first;

      } else {
        assert(!"unhandled simple type");
      }

      *this << ")";
    }
  }

  void visit_range(const Range &) final {
    *this << numeric_type;

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

    // do we need to propagate parameters passed in by value to return values?
    if (!to_return.empty()) {
      // use a block so anything containing sees us as a single statement
      *this << tab() << "{\n";
      indent();
      for (const std::string &s : to_return)
        *this << tab() << "__" << s << " = " << s << ";\n";
    }

    // only relevant if an actual value is being returned
    if (n.expr != nullptr)
      *this << tab() << "__return = " << *n.expr << ";\n";

    if (!to_return.empty()) {
      dedent();
      *this << tab() << "}\n";
    }
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
      size_t j __attribute__((unused)) =
          params.size() - n.quantifiers.size() + i;
      assert(params[j] == &n.quantifiers[i] &&
             "ruleset parameter management out of sync");
    }

    // remove the parameters as we ascend
    params.resize(params.size() - n.quantifiers.size());
  }

  void visit_scalarset(const Scalarset &) final {
    *this << numeric_type;

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

    if (n.expr->is_pure()) {
      *this << tab() << "case\n";
      indent();

      for (const SwitchCase &c : n.cases) {
        emit_leading_comments(c);
        if (c.matches.empty()) {
          *this << tab() << "default";
        } else {
          *this << tab();
          std::string sep;
          for (const Ptr<Expr> &m : c.matches) {
            *this << sep << *n.expr << " == " << *m;
            sep = " || ";
          }
        }
        *this << " : " << c;
      }

      dedent();
      *this << tab() << "esac\n";

    } else {
      // this switch is on an expression with side effects, so we need to first
      // store it in a temporary which we will then emit multiple times
      std::string expr = make_symbol("");

      *this << tab() << "{\n";
      indent();

      *this << tab() << "var " << expr << " : " << *n.expr->type() << ";\n"
            << tab() << expr << " = " << *n.expr;

      *this << tab() << "case\n";
      indent();

      for (const SwitchCase &c : n.cases) {
        emit_leading_comments(c);
        if (c.matches.empty()) {
          *this << tab() << "default";
        } else {
          *this << tab();
          std::string sep;
          for (const Ptr<Expr> &m : c.matches) {
            *this << sep << expr << " == " << *m;
            sep = " || ";
          }
        }
        *this << " : " << c;
      }

      dedent();
      *this << tab() << "esac\n";

      dedent();
      *this << tab() << "}\n";
    }
  }

  void visit_switchcase(const SwitchCase &n) final {

    // the match itself will have been handled by our parent (Switch)

    *this << "{\n";
    indent();

    for (const Ptr<Stmt> &s : n.body) {
      emit_leading_comments(*s);
      *this << *s;
    }

    dedent();
    *this << tab() << "}\n";
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
        *this << numeric_type;
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
