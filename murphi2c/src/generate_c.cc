#include <cassert>
#include <cstddef>
#include "generate_c.h"
#include <gmpxx.h>
#include <iostream>
#include <rumur/rumur.h>
#include <string>
#include <utility>
#include <vector>

using namespace rumur;

static std::string escape(const std::string &s) {
  // TODO: use ../../rumur/src/utils.cc:escape()
  return s;
}

namespace {

class CGenerator : public ConstBaseTraversal {

 private:
  std::ostream &out;
  size_t indent_level = 0;

 public:
  explicit CGenerator(std::ostream &out_): out(out_) { }

  // helpers to make output below more natural

  CGenerator &operator<<(const std::string &s) {
    out << s;
    return *this;
  }

  CGenerator &operator<<(const Node &n) {
    dispatch(n);
    return *this;
  }

  void visit_add(const Add &n) final {
    *this << "(" << *n.lhs << " + " << *n.rhs << ")";
  }

  void visit_aliasdecl(const AliasDecl &n) final {
    *this << "#define " << n.name << " " << *n.value << "\n";
  }

  void visit_aliasrule(const AliasRule&) final {
    // this is unreachable because generate_c is only ever called with a Model
    // and visit_model flattens all rules
    assert(!"unreachable");
    __builtin_unreachable();
  }

  void visit_aliasstmt(const AliasStmt &n) final {
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

  void visit_and(const And &n) final {
    *this << "(" << *n.lhs << " && " << *n.rhs << ")";
  }

  void visit_array(const Array &n) final {
    mpz_class count = n.index_type->count();

    assert(count > 0 && "index type of array does not include undefined");
    count--;

    // wrap the array in a struct so that we do not have the awkwardness of
    // having to emit its type and size on either size of another node
    *this << "struct { " << *n.element_type << " data[" << count.get_str()
      << "]; }";
  }

  void visit_assignment(const Assignment &n) final {
    *this << indentation() << *n.lhs << " = " << *n.rhs << ";\n";
  }

  void visit_clear(const Clear &n) final {
    *this << indentation() << "memset(&" << *n.rhs << ", 0, sizeof(" << *n.rhs
      << "));\n";
  }

  void visit_constdecl(const ConstDecl &n) final {
    *this << indentation() << "const ";
    if (n.type == nullptr) {
      *this << "__auto_type";
    } else {
      *this << *n.type;
    }
    *this << " " << n.name << " = " << *n.value << ";\n";
  }

  void visit_div(const Div &n) final {
    *this << "(" << *n.lhs << " / " << *n.rhs << ")";
  }

  void visit_element(const Element &n) final {
    *this << "(" << *n.array << ".data[" << *n.index << "])";
  }

  void visit_enum(const Enum &n) final {
    *this << "enum { ";
    for (const std::pair<std::string, location> &m : n.members) {
      *this << m.first << ", ";
    }
    *this << "}";
  }

  void visit_eq(const Eq &n) final {
    *this << "(" << *n.lhs << " == " << *n.rhs << ")";
  }

  void visit_errorstmt(const ErrorStmt &n) final {
    *this << "error(\"" << escape(n.message) << "\")";
  }

  void visit_exists(const Exists &n) final {
    *this << "({ bool res_ = false; " << n.quantifier << " { res_ |= "
      << *n.expr << "; } res_; })";
  }

  void visit_exprid(const ExprID &n) final {
    *this << "(" << n.id << ")";
  }

  void visit_field(const Field &n) final {
    *this << "(" << *n.record << "." << n.field << ")";
  }

  void visit_for(const For &n) final {
    *this << indentation() << n.quantifier << " {\n";
    indent();
    for (const Ptr<Stmt> &s : n.body) {
      *this << *s;
    }
    dedent();
    *this << indentation() << "}\n";
  }

  void visit_forall(const Forall &n) final {
    *this << "({ bool res_ = true; " << n.quantifier << " { res_ &= "
      << *n.expr << "; } res_; })";
  }

  void visit_function(const Function &n) final {
    *this << indentation();
    if (n.return_type == nullptr) {
      *this << "void";
    } else {
      *this << *n.return_type;
    }
    *this << " " << n.name << "(";
    bool first = true;
    for (const Ptr<VarDecl> &p : n.parameters) {
      if (!first) {
        *this << ", ";
      }
      *this << *p->type << " ";
      // if this is a var parameter, it needs to be a pointer
      if (!p->readonly) {
        *this << "*" << p->name << "_";
      } else {
        *this << p->name;
      }
      first = false;
    }
    *this << ") {\n";
    indent();
    // provide aliases of var parameters under their original name
    for (const Ptr<VarDecl> &p : n.parameters) {
      if (!p->readonly) {
        *this << "#define " << p->name << " (*" << p->name << "_)\n";
      }
    }
    for (const Ptr<Decl> &d : n.decls) {
      *this << *d;
    }
    for (const Ptr<Stmt> &s : n.body) {
      *this << *s;
    }
    // clean up var aliases
    for (const Ptr<VarDecl> &p : n.parameters) {
      if (!p->readonly) {
        *this << "#undef " << p->name << "\n";
      }
    }
    dedent();
    *this << "}\n";
  }

  void visit_functioncall(const FunctionCall &n) final {
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

  void visit_geq(const Geq &n) final {
    *this << "(" << *n.lhs << " >= " << *n.rhs << ")";
  }

  void visit_gt(const Gt &n) final {
    *this << "(" << *n.lhs << " > " << *n.rhs << ")";
  }

  void visit_if(const If &n) final {
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

  void visit_ifclause(const IfClause &n) final {
    if (n.condition != nullptr) {
      *this << "if " << *n.condition << " ";
    }
    *this << "{\n";
    indent();
    for (const Ptr<Stmt> &s : n.body) {
      *this << *s;
    }
    dedent();
    *this << indentation() << "}";
  }

  void visit_implication(const Implication &n) final {
    *this << "(!" << *n.lhs << " || " << *n.rhs << ")";
  }

  void visit_isundefined(const IsUndefined&) final {
    // TODO: check and prevent instances of this from appearing at this point
  }

  void visit_leq(const Leq &n) final {
    *this << "(" << *n.lhs << " <= " << *n.rhs << ")";
  }

  void visit_lt(const Lt &n) final {
    *this << "(" << *n.lhs << " < " << *n.rhs << ")";
  }

  void visit_mod(const Mod &n) final {
    *this << "(" << *n.lhs << " % " << *n.rhs << ")";
  }

  void visit_model(const Model &n) final {

    // constants, types and variables
    for (const Ptr<Decl> &d : n.decls) {
      *this << *d;
    }

    *this << "\n";

    // functions and procedures
    for (const Ptr<Function> &f : n.functions) {
      *this << *f << "\n";
    }

    // flatten the rules so we do not have to deal with the hierarchy of
    // rulesets, aliasrules, etc.
    std::vector<Ptr<Rule>> flattened;
    for (const Ptr<Rule> &r : n.rules) {
      std::vector<Ptr<Rule>> rs = r->flatten();
      flattened.insert(flattened.end(), rs.begin(), rs.end());
    }

    // startstates, rules, invariants
    for (const Ptr<Rule> &r : flattened) {
      *this << *r << "\n";
    }
  }

  void visit_mul(const Mul &n) final {
    *this << "(" << *n.lhs << " * " << *n.rhs << ")";
  }

  void visit_negative(const Negative &n) final {
    *this << "(-" << *n.rhs << ")";
  }

  void visit_neq(const Neq &n) final {
    *this << "(" << *n.lhs << " != " << *n.rhs << ")";
  }

  void visit_not(const Not &n) final {
    *this << "(!" << *n.rhs << ")";
  }

  void visit_number(const Number &n) final {
    *this << "(" << n.value.get_str() << ")";
  }

  void visit_or(const Or &n) final {
    *this << "(" << *n.lhs << " || " << *n.rhs << ")";
  }

  void visit_procedurecall(const ProcedureCall &n) final {
    *this << indentation() << n.call << ";\n";
  }

  void visit_property(const Property&) final {
    // this is unreachable because generate_c is only ever called with a Model
    // and nothing that contains a Property descends into it
    assert(!"unreachable");
    __builtin_unreachable();
  }

  void visit_propertyrule(const PropertyRule &n) final {

    // function prototype
    *this << indentation() << "bool " << n.name << "(";

    // parameters
    bool first = true;
    for (const Quantifier &q : n.quantifiers) {
      if (!first) {
        *this << ", ";
      }
      if (auto t = dynamic_cast<const TypeExprID*>(q.type.get())) {
        *this << t->name;
      } else {
        *this << "int64_t";
      }
      *this << " " << q.name;
      first = false;
    }

    *this << ") {\n";
    indent();

    // any aliases this property uses
    for (const Ptr<AliasDecl> &a : n.aliases) {
      *this << *a;
    }

    *this << indentation() << "return " << *n.property.expr << ";\n";

    // clean up any aliases we defined
    for (const Ptr<AliasDecl> &a : n.aliases) {
      *this << "#undef " << a->name << "\n";
    }

    dedent();
    *this << "}\n";
  }

  void visit_propertystmt(const PropertyStmt &n) final {

    switch (n.property.category) {

      case Property::ASSERTION:
        *this << indentation() << "if (!" << *n.property.expr << ") {\n";
        indent();
        *this << indentation() << "if (failed_assertion != NULL) {\n";
        indent();
        *this << indentation() << "failed_assertion(\""
          << escape(n.message == "" ? n.property.expr->to_string() : n.message)
          << "\");\n";
        dedent();
        *this << indentation() << "}\n";
        dedent();
        *this << indentation() << "}\n";
        break;

      case Property::ASSUMPTION:
        *this << indentation() << "if (!" << *n.property.expr << ") {\n";
        indent();
        *this << indentation() << "if (failed_assumption != NULL) {\n";
        indent();
        *this << indentation() << "failed_assumption(\""
          << escape(n.message == "" ? n.property.expr->to_string() : n.message)
          << "\");\n";
        dedent();
        *this << indentation() << "}\n";
        dedent();
        *this << indentation() << "}\n";
        break;

      case Property::COVER:
        *this << indentation() << "if " << *n.property.expr << " {\n";
        indent();
        *this << indentation() << "if (cover != NULL) {\n";
        indent();
        *this << indentation() << "cover(\""
          << escape(n.message == "" ? n.property.expr->to_string() : n.message)
          << "\");\n";
        dedent();
        *this << indentation() << "}\n";
        dedent();
        *this << indentation() << "}\n";
        break;

      case Property::LIVENESS:
        *this << indentation() << "if " << *n.property.expr << " {\n";
        indent();
        *this << indentation() << "if (liveness != NULL) {\n";
        indent();
        *this << indentation() << "liveness(\""
          << escape(n.message == "" ? n.property.expr->to_string() : n.message)
          << "\");\n";
        dedent();
        *this << indentation() << "}\n";
        dedent();
        *this << indentation() << "}\n";
        break;

    }
  }

  void visit_put(const Put &n) final {
    *this << indentation() << "printf(";
    if (n.expr == nullptr) {
      *this << "\"%s\\n\", \"" << escape(n.value) << "\")";
    } else {
      *this << "\"%\" PRId64 \"\\n\", " << *n.expr << ")";
    }
    *this << ";\n";
  }

  void visit_quantifier(const Quantifier &n) final {

    if (n.type == nullptr) {
      bool down_count = n.from->constant() && n.to->constant()
        && n.to->constant_fold() < n.from->constant_fold();
      *this << "for (int64_t " << n.name << " = " << *n.from << "; " << n.name
        << " " << (down_count ? ">=" : "<=") << "; " << n.name << " += ";
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
      if (!e->members.empty()) {
        *this << "for (__auto_type " << n.name << " = " << e->members[0].first
          << "; " << n.name << " <= " << e->members[e->members.size() - 1].first
          << "; " << n.name << "++)";
      }
      return;
    }

    if (auto r = dynamic_cast<const Range*>(resolved.get())) {
      *this << "for (int64_t " << n.name << " = " << *r->min << "; " << n.name
        << " <= " << *r->max << "; " << n.name << "++)";
      return;
    }

    if (auto s = dynamic_cast<const Scalarset*>(resolved.get())) {
      *this << "for (int64_t " << n.name << " = 0; " << n.name << " <= "
        << *s->bound << "; " << n.name << "++)";
      return;
    }

    assert(!"missing case in visit_quantifier()");
  }

  void visit_range(const Range&) final {
    *this << "int64_t";
  }

  void visit_record(const Record &n) final {
    *this << "struct {\n";
    indent();
    for (const Ptr<VarDecl> &f : n.fields) {
      *this << indentation() << *f << ";\n";
    }
    dedent();
    *this << indentation() << "}";
  }

  void visit_return(const Return &n) final {
    *this << indentation() << "return";
    if (n.expr != nullptr) {
      *this << " " << *n.expr;
    }
    *this << ";\n";
  }

  void visit_ruleset(const Ruleset&) final {
    // this is unreachable because generate_c is only ever called with a Model
    // and all rule are flattened during visit_model
    assert(!"unreachable");
    __builtin_unreachable();
  }

  void visit_scalarset(const Scalarset&) final {
    *this << "int64_t";
  }

  void visit_simplerule(const SimpleRule &n) final {
    // TODO: rules with non-symbol names
    *this << indentation() << "bool guard_" << n.name << "(";

    // parameters
    bool first = true;
    for (const Quantifier &q : n.quantifiers) {
      if (!first) {
        *this << ", ";
      }
      if (auto t = dynamic_cast<const TypeExprID*>(q.type.get())) {
        *this << t->name;
      } else {
        *this << "int64_t";
      }
      *this << " " << q.name;
      first = false;
    }

    *this << ") {\n";
    indent();

    // any aliases that are defined in an outer scope
    for (const Ptr<AliasDecl> &a : n.aliases) {
      *this << *a;
    }

    *this << indentation() << "return ";
    if (n.guard == nullptr) {
      *this << "true";
    } else {
      *this << *n.guard;
    }
    *this << ";\n";

    // clean up aliases
    for (const Ptr<AliasDecl> &a : n.aliases) {
      *this << "#undef " << a->name << "\n";
    }

    dedent();
    *this << indentation() << "}\n\n";

    *this << indentation() << "void rule_" << n.name << "(";

    // parameters
    first = true;
    for (const Quantifier &q : n.quantifiers) {
      if (!first) {
        *this << ", ";
      }
      if (auto t = dynamic_cast<const TypeExprID*>(q.type.get())) {
        *this << t->name;
      } else {
        *this << "int64_t";
      }
      *this << " " << q.name;
      first = false;
    }

    *this << ") {\n";
    indent();

    // aliases, variables, local types, etc.
    for (const Ptr<AliasDecl> &a : n.aliases) {
      *this << *a;
    }
    for (const Ptr<Decl> &d : n.decls) {
      *this << *d;
    }

    for (const Ptr<Stmt> &s : n.body) {
      *this << *s;
    }

    // clean up any aliases we defined
    for (const Ptr<Decl> &d : n.decls) {
      if (auto a = dynamic_cast<const AliasDecl*>(d.get())) {
        *this << "#undef " << a->name << "\n";
      }
    }
    for (const Ptr<AliasDecl> &a : n.aliases) {
      *this << "#undef " << a->name << "\n";
    }

    dedent();
    *this << indentation() << "}\n";
  }

  void visit_startstate(const StartState &n) final {
    // TODO: startstates with non-symbol names
    *this << indentation() << "void startstate_" << n.name << "(";

    // parameters
    bool first = true;
    for (const Quantifier &q : n.quantifiers) {
      if (!first) {
        *this << ", ";
      }
      if (auto t = dynamic_cast<const TypeExprID*>(q.type.get())) {
        *this << t->name;
      } else {
        *this << "int64_t";
      }
      *this << " " << q.name;
      first = false;
    }

    *this << ") {\n";
    indent();

    // aliases, variables, local types, etc.
    for (const Ptr<AliasDecl> &a : n.aliases) {
      *this << *a;
    }
    for (const Ptr<Decl> &d : n.decls) {
      *this << *d;
    }

    for (const Ptr<Stmt> &s : n.body) {
      *this << *s;
    }

    // clean up any aliases we defined
    for (const Ptr<Decl> &d : n.decls) {
      if (auto a = dynamic_cast<const AliasDecl*>(d.get())) {
        *this << "#undef " << a->name << "\n";
      }
    }
    for (const Ptr<AliasDecl> &a : n.aliases) {
      *this << "#undef " << a->name << "\n";
    }

    dedent();
    *this << indentation() << "}\n\n";
  }

  void visit_sub(const Sub &n) final {
    *this << "(" << *n.lhs << " - " << *n.rhs << ")";
  }

  void visit_switch(const Switch &n) final {
    // TODO: handle cases where expression or cases are not primitives
    *this << indentation() << "switch " << *n.expr << " {\n\n";
    indent();
    for (const SwitchCase &c : n.cases) {
      *this << c << "\n";
    }
    dedent();
    *this << indentation() << "}\n";
  }

  void visit_switchcase(const SwitchCase &n) final {
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

  void visit_ternary(const Ternary &n) final {
    *this << "(" << *n.cond << " ? " << *n.lhs << " : " << *n.rhs << ")";
  }

  void visit_typedecl(const TypeDecl &n) final {
    *this << indentation() << "typedef " << *n.value << " " << n.name << ";\n";
  }

  void visit_typeexprid(const TypeExprID &n) final {
    *this << n.name;
  }

  void visit_undefine(const Undefine &n) final {
    *this << indentation() << "memset(&" << *n.rhs << ", 0, sizeof(" << *n.rhs
      << "));\n";
  }

  void visit_vardecl(const VarDecl &n) final {
    *this << indentation() << *n.type << " " << n.name << ";\n";
  }

  void visit_while(const While &n) final {
    *this << indentation() << "while " << *n.condition << " {\n";
    indent();
    for (const Ptr<Stmt> &s : n.body) {
      *this << *s;
    }
    dedent();
    *this << indentation() << "}\n";
  }

  virtual ~CGenerator() = default;

 private:
  std::string indentation() const {
    return std::string(indent_level * 2, ' ');
  }

  void indent() {
    indent_level++;
  }

  void dedent() {
    assert(indent_level > 0 && "attempted negative indentation");
    indent_level--;
  }
};

}

void generate_c(const Node &n, std::ostream &out) {

  // standard support we will assume is available in the code emitted above
  out << "#include <stdbool.h>\n"
         "#include <stddef.h>\n"
         "#include <stdint.h>\n"
         "#include <stdlib.h>\n"
         "#include <string.h>\n";

  // emit prototypes for functions we anticipate the user might provide
  out << "void failed_assertion(const char *message) __attribute__((weak));\n"
         "void failed_assumption(const char *message) __attribute__((weak));\n"
         "void error(const char *message) __attribute__((weak));\n"
         "void cover(const char *message) __attribute__((weak));\n"
         "void liveness(const char *message) __attribute__((weak));\n";

  CGenerator gen(out);
  gen.dispatch(n);
}
