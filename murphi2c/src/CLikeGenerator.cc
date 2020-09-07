#include <cstddef>
#include <cassert>
#include "CLikeGenerator.h"
#include "../../common/escape.h"
#include <gmpxx.h>
#include "options.h"
#include <rumur/rumur.h>
#include <string>
#include <utility>
#include <vector>

using namespace rumur;

void CLikeGenerator::visit_add(const Add &n) {
  *this << "(" << *n.lhs << " + " << *n.rhs << ")";
}

void CLikeGenerator::visit_aliasdecl(const AliasDecl &n) {
  *this << "#define " << n.name << " " << *n.value << "\n";
}

void CLikeGenerator::visit_aliasrule(const AliasRule&) {
  // this is unreachable because generate_c is only ever called with a Model
  // and visit_model flattens all rules
  assert(!"unreachable");
  __builtin_unreachable();
}

void CLikeGenerator::visit_aliasstmt(const AliasStmt &n) {
  for (const Ptr<AliasDecl> &a : n.aliases) {
    *this << *a;
  }
  for (const Ptr<Stmt> &s : n.body) {
    *this << *s;
  }
  for (const Ptr<AliasDecl> &a : n.aliases) {
    *this << "#undef " << a->name << "\n";
  }
}

void CLikeGenerator::visit_and(const And &n) {
  *this << "(" << *n.lhs << " && " << *n.rhs << ")";
}

void CLikeGenerator::visit_array(const Array &n) {
  mpz_class count = n.index_type->count();

  assert(count > 0 && "index type of array does not include undefined");
  count--;

  // wrap the array in a struct so that we do not have the awkwardness of
  // having to emit its type and size on either size of another node
  *this << "struct " << (pack ? "__attribute__((packed)) " : "") << "{ "
    << *n.element_type << " data[" << count.get_str() << "];";

  // The index for this array may be an enum declared inline:
  //
  //   array [enum {A, B}] of foo
  //
  // If so, we need to emit it somehow so that the enum’s members can be
  // referenced later. We define it within this struct to avoid any awkward
  // lexical issues.
  if (auto e = dynamic_cast<const Enum*>(n.index_type.get())) {
    *this << " " << *e << ";";
  }

   *this <<" }";
}

void CLikeGenerator::visit_assignment(const Assignment &n) {
  *this << indentation() << *n.lhs << " = " << *n.rhs << ";\n";
}

void CLikeGenerator::visit_band(const Band &n) {
  *this << "(" << *n.lhs << " & " << *n.rhs << ")";
}

void CLikeGenerator::visit_bnot(const Bnot &n) {
  *this << "(~" << *n.rhs << ")";
}

void CLikeGenerator::visit_bor(const Bor &n) {
  *this << "(" << *n.lhs << " | " << *n.rhs << ")";
}

void CLikeGenerator::visit_clear(const Clear &n) {
  *this << indentation() << "memset(&" << *n.rhs << ", 0, sizeof(" << *n.rhs
    << "));\n";
}

void CLikeGenerator::visit_div(const Div &n) {
  *this << "(" << *n.lhs << " / " << *n.rhs << ")";
}

void CLikeGenerator::visit_element(const Element &n) {
  *this << "(" << *n.array << ".data[" << *n.index << "])";
}

void CLikeGenerator::visit_enum(const Enum &n) {
  *this << "enum { ";
  for (const std::pair<std::string, location> &m : n.members) {
    *this << m.first << ", ";
  }
  *this << "}";
}

void CLikeGenerator::visit_eq(const Eq &n) {

  if (!n.lhs->type()->is_simple()) {
    // This is a comparison of an array or struct. We cannot use the built-in
    // == operator, so we use memcmp. This only works if all members are
    // packed, hence why `__attribute__((pack))` is emitted in other places.
    assert(pack && "comparison of complex types is present but structures "
      "are not packed");
    *this << "(memcmp(&" << *n.lhs << ", &" << *n.rhs << ", sizeof" << *n.lhs
      << ") == 0)";

    return;
  }

  *this << "(" << *n.lhs << " == " << *n.rhs << ")";
}

void CLikeGenerator::visit_errorstmt(const ErrorStmt &n) {
  *this << indentation() << "error(\"" << escape(n.message) << "\");\n";
}

void CLikeGenerator::visit_exists(const Exists &n) {
  *this << "({ bool res_ = false; " << n.quantifier << " { res_ |= "
    << *n.expr << "; } res_; })";
}

void CLikeGenerator::visit_exprid(const ExprID &n) {
  *this << "(" << n.id << ")";
}

void CLikeGenerator::visit_field(const Field &n) {
  *this << "(" << *n.record << "." << n.field << ")";
}

void CLikeGenerator::visit_for(const For &n) {

  // open a scope to make all of this appear as a single statement to any
  // enclosing code
  *this << indentation() << "do {\n";
  indent();

  // if the type of the quantifier is an enum defined inline, we need to
  // define this in advance because C does not permit this to be defined
  // within the for loop initialiser
  if (auto e = dynamic_cast<const Enum*>(n.quantifier.type.get())) {
    *this << indentation() << *e << ";\n";
  }

  *this << indentation() << n.quantifier << " {\n";
  indent();
  for (const Ptr<Stmt> &s : n.body) {
    *this << *s;
  }
  dedent();
  *this << indentation() << "}\n";

  dedent();
  *this << indentation() << "} while (0);\n";
}

void CLikeGenerator::visit_forall(const Forall &n) {

  // open a GNU statement expression
  *this << "({ ";

  // see corresponding logic in visit_for() for an explanation
  if (auto e = dynamic_cast<const Enum*>(n.quantifier.type.get())) {
    *this << *e << "; ";
  }

  *this << "bool res_ = true; " << n.quantifier << " { res_ &= "
    << *n.expr << "; } res_; })";
}

void CLikeGenerator::visit_functioncall(const FunctionCall &n) {
  *this << n.name << "(";
  assert(n.function != nullptr && "unresolved function call in AST");
  auto it = n.function->parameters.begin();
  bool first = true;
  for (const Ptr<Expr> &a : n.arguments) {
    if (!first) {
      *this << ", ";
    }
    if (!(*it)->readonly) {
      *this << "&";
    }
    *this << *a;
    first = false;
    it++;
  }
  *this << ")";
}

void CLikeGenerator::visit_geq(const Geq &n) {
  *this << "(" << *n.lhs << " >= " << *n.rhs << ")";
}

void CLikeGenerator::visit_gt(const Gt &n) {
  *this << "(" << *n.lhs << " > " << *n.rhs << ")";
}

void CLikeGenerator::visit_if(const If &n) {
  bool first = true;
  for (const IfClause &c : n.clauses) {
    if (first) {
      *this << indentation();
    } else {
      *this << " else ";
    }
    dispatch(c);
    first = false;
  }
  *this << "\n";
}

void CLikeGenerator::visit_ifclause(const IfClause &n) {
  if (n.condition != nullptr) {

    // we do not need to emit surrounding brackets for binary expressions
    // because they are already emitted with brackets
    bool needs_bracketing
      = dynamic_cast<const BinaryExpr*>(n.condition.get()) == nullptr;

    *this << "if ";
    if (needs_bracketing)
      *this << "(";
    *this << *n.condition;
    if (needs_bracketing)
      *this << ")";
    *this << " ";
  }
  *this << "{\n";
  indent();
  for (const Ptr<Stmt> &s : n.body) {
    *this << *s;
  }
  dedent();
  *this << indentation() << "}";
}

void CLikeGenerator::visit_implication(const Implication &n) {
  *this << "(!" << *n.lhs << " || " << *n.rhs << ")";
}

void CLikeGenerator::visit_isundefined(const IsUndefined&) {
  // check() prevents a model with isundefined expressions from making it
  // through to here
  assert(!"unreachable");
  __builtin_unreachable();
}

void CLikeGenerator::visit_leq(const Leq &n) {
  *this << "(" << *n.lhs << " <= " << *n.rhs << ")";
}

void CLikeGenerator::visit_lsh(const Lsh &n) {
  *this << "(" << *n.lhs << " << " << *n.rhs << ")";
}

void CLikeGenerator::visit_lt(const Lt &n) {
  *this << "(" << *n.lhs << " < " << *n.rhs << ")";
}

void CLikeGenerator::visit_mod(const Mod &n) {
  *this << "(" << *n.lhs << " % " << *n.rhs << ")";
}

void CLikeGenerator::visit_model(const Model &n) {

  for (const Ptr<Node> &c : n.children) {

    // if this is a rule, first flatten it so we do not have to deal with the
    // hierarchy of rulesets, aliasrules, etc.
    if (auto r = dynamic_cast<const Rule*>(c.get())) {
      std::vector<Ptr<Rule>> rs = r->flatten();
      for (const Ptr<Rule> &r2 : rs)
        *this << *r2 << "\n";

    } else {
      *this << *c << "\n";
    }
  }
}

void CLikeGenerator::visit_mul(const Mul &n) {
  *this << "(" << *n.lhs << " * " << *n.rhs << ")";
}

void CLikeGenerator::visit_negative(const Negative &n) {
  *this << "(-" << *n.rhs << ")";
}

void CLikeGenerator::visit_neq(const Neq &n) {

  if (!n.lhs->type()->is_simple()) {
    // see explanation in visit_eq()
    assert(pack && "comparison of complex types is present but structures "
      "are not packed");
    *this << "(memcmp(&" << *n.lhs << ", &" << *n.rhs << ", sizeof" << *n.lhs
      << ") != 0)";

    return;
  }

  *this << "(" << *n.lhs << " != " << *n.rhs << ")";
}

void CLikeGenerator::visit_not(const Not &n) {
  *this << "(!" << *n.rhs << ")";
}

void CLikeGenerator::visit_number(const Number &n) {
  *this << "((" << value_type << ")(" << n.value.get_str() << "))";
}

void CLikeGenerator::visit_or(const Or &n) {
  *this << "(" << *n.lhs << " || " << *n.rhs << ")";
}

void CLikeGenerator::visit_procedurecall(const ProcedureCall &n) {
  *this << indentation() << n.call << ";\n";
}

void CLikeGenerator::visit_property(const Property&) {
  // this is unreachable because generate_c is only ever called with a Model
  // and nothing that contains a Property descends into it
  assert(!"unreachable");
  __builtin_unreachable();
}

void CLikeGenerator::visit_propertystmt(const PropertyStmt &n) {

  switch (n.property.category) {

    case Property::ASSERTION:
      *this << indentation() << "if (!" << *n.property.expr << ") {\n";
      indent();
      *this << indentation() << "failed_assertion(\""
        << escape(n.message == "" ? n.property.expr->to_string() : n.message)
        << "\");\n";
      dedent();
      *this << indentation() << "}\n";
      break;

    case Property::ASSUMPTION:
      *this << indentation() << "if (!" << *n.property.expr << ") {\n";
      indent();
      *this << indentation() << "failed_assumption(\""
        << escape(n.message == "" ? n.property.expr->to_string() : n.message)
        << "\");\n";
      dedent();
      *this << indentation() << "}\n";
      break;

    case Property::COVER:
      *this << indentation() << "if " << *n.property.expr << " {\n";
      indent();
      *this << indentation() << "cover(\""
        << escape(n.message == "" ? n.property.expr->to_string() : n.message)
        << "\");\n";
      dedent();
      *this << indentation() << "}\n";
      break;

    case Property::LIVENESS:
      *this << indentation() << "if " << *n.property.expr << " {\n";
      indent();
      *this << indentation() << "liveness(\""
        << escape(n.message == "" ? n.property.expr->to_string() : n.message)
        << "\");\n";
      dedent();
      *this << indentation() << "}\n";
      break;

  }
}

void CLikeGenerator::visit_put(const Put &n) {
  *this << indentation();
  if (n.expr == nullptr) {
    *this << "printf(\"%s\\n\", \"" << escape(n.value) << "\")";
  } else {
    *this << "print" << value_type << "(" << *n.expr << ")";
  }
  *this << ";\n";
}

void CLikeGenerator::visit_quantifier(const Quantifier &n) {

  if (n.type == nullptr) {
    bool down_count = n.from->constant() && n.to->constant()
      && n.to->constant_fold() < n.from->constant_fold();
    *this << "for (" << value_type << " " << n.name << " = " << *n.from << "; "
      << n.name << " " << (down_count ? ">=" : "<=") << " " << *n.to << "; "
      << n.name << " += ";
    if (n.step == nullptr) {
      *this << "1";
    } else {
      *this << *n.step;
    }
    *this << ")";
    return;
  }

  const Ptr<TypeExpr> resolved = n.type->resolve();

  if (auto e = dynamic_cast<const Enum*>(resolved.get())) {
    if (e->members.empty()) {
      // degenerate loop
      *this << "for (int " << n.name << " = 0; " << n.name << " < 0; "
        << n.name << "++)";
    } else {
      // common case
      *this << "for (__typeof__(" << e->members[0].first << ") " << n.name
        << " = " << e->members[0].first << "; " << n.name << " <= "
        << e->members[e->members.size() - 1].first << "; " << n.name << "++)";
    }
    return;
  }

  if (auto r = dynamic_cast<const Range*>(resolved.get())) {
    *this << "for (" << value_type << " " << n.name << " = " << *r->min << "; "
      << n.name << " <= " << *r->max << "; " << n.name << "++)";
    return;
  }

  if (auto s = dynamic_cast<const Scalarset*>(resolved.get())) {
    *this << "for (" << value_type << " " << n.name << " = 0; " << n.name
      << " <= " << *s->bound << "; " << n.name << "++)";
    return;
  }

  assert(!"missing case in visit_quantifier()");
}

void CLikeGenerator::visit_range(const Range&) {
  *this << value_type;
}

void CLikeGenerator::visit_record(const Record &n) {
  *this << "struct " << (pack ? "__attribute__((packed)) " : "") << "{\n";
  indent();
  for (const Ptr<VarDecl> &f : n.fields) {
    *this << *f;
  }
  dedent();
  *this << indentation() << "}";
}

void CLikeGenerator::visit_return(const Return &n) {
  *this << indentation() << "return";
  if (n.expr != nullptr) {
    *this << " " << *n.expr;
  }
  *this << ";\n";
}

void CLikeGenerator::visit_rsh(const Rsh &n) {
  *this << "(" << *n.lhs << " >> " << *n.rhs << ")";
}

void CLikeGenerator::visit_ruleset(const Ruleset&) {
  // this is unreachable because generate_c is only ever called with a Model
  // and all rule are flattened during visit_model
  assert(!"unreachable");
  __builtin_unreachable();
}

void CLikeGenerator::visit_scalarset(const Scalarset&) {
  *this << value_type;
}

void CLikeGenerator::visit_sub(const Sub &n) {
  *this << "(" << *n.lhs << " - " << *n.rhs << ")";
}

void CLikeGenerator::visit_switch(const Switch &n) {

  // Rumur permits switch statements with non-constant case expressions, while
  // C’s switch statements do not support this. To deal with this discrepancy,
  // we emit switch statements as more flexible if-then-else blocks instead.

  // make the variable declaration and if-then-else block appear as a single
  // statement to any enclosing code
  *this << indentation() << "do {\n";
  indent();

  // we need to declare a temporary for the expression here because it may not
  // be pure, so we cannot necessarily safely emit it repeatedly in the
  // if-then-else conditions
  *this << indentation() << "__typeof__(" << *n.expr << ") res_ = " << *n.expr
    << ";\n";

  bool first = true;
  for (const SwitchCase &c : n.cases) {
    if (first) {
      *this << indentation();
    } else {
      *this << " else ";
    }
    if (!c.matches.empty()) {
      std::string sep = "";
      *this << "if (";
      for (const Ptr<Expr> &m : c.matches) {
        *this << sep << "res_ == " << *m;
        sep = " || ";
      }
      *this << ") ";
    }
    *this << "{\n";
    indent();
    for (const Ptr<Stmt> &s : c.body) {
      *this << *s;
    }
    dedent();
    *this << indentation() << "}";
    first = false;
  }
  *this << "\n";

  dedent();
  *this << indentation() << "} while (0);\n";
}

void CLikeGenerator::visit_switchcase(const SwitchCase &n) {
  if (n.matches.empty()) {
    *this << indentation() << "default:\n";
  } else {
    for (const Ptr<Expr> &m : n.matches) {
      *this << indentation() << "case " << *m << ":\n";
    }
  }
  indent();
  for (const Ptr<Stmt> &s : n.body) {
    *this << *s;
  }
  *this << indentation() << "break;\n";
  dedent();
}

void CLikeGenerator::visit_ternary(const Ternary &n) {
  *this << "(" << *n.cond << " ? " << *n.lhs << " : " << *n.rhs << ")";
}

void CLikeGenerator::visit_typedecl(const TypeDecl &n) {

  // If we are typedefing something that is an enum, save this for later lookup.
  // See CGenerator/HGenerator::visit_constdecl for the purpose of this.
  if (auto e = dynamic_cast<const Enum*>(n.value.get()))
    enum_typedefs[e->unique_id] = n.name;

  *this << indentation() << "typedef " << *n.value << " " << n.name << ";\n";
}

void CLikeGenerator::visit_typeexprid(const TypeExprID &n) {
  *this << n.name;
}

void CLikeGenerator::visit_undefine(const Undefine &n) {
  *this << indentation() << "memset(&" << *n.rhs << ", 0, sizeof(" << *n.rhs
    << "));\n";
}

void CLikeGenerator::visit_while(const While &n) {
  *this << indentation() << "while " << *n.condition << " {\n";
  indent();
  for (const Ptr<Stmt> &s : n.body) {
    *this << *s;
  }
  dedent();
  *this << indentation() << "}\n";
}

void CLikeGenerator::visit_xor(const Xor &n) {
  *this << "(" << *n.lhs << " ^ " << *n.rhs << ")";
}

CLikeGenerator &CLikeGenerator::operator<<(const std::string &s) {
  out << s;
  return *this;
}

CLikeGenerator &CLikeGenerator::operator<<(const Node &n) {
  dispatch(n);
  return *this;
}

CLikeGenerator::~CLikeGenerator() { }
