#include <iostream>
#include "Printer.h"
#include <rumur/rumur.h>
#include <string>

using namespace rumur;

Printer::Printer(std::istream &in_, std::ostream &out_): in(in_), out(out_) { }

void Printer::visit_add(const Add &n) {
  visit_bexpr("add", n);
}

void Printer::visit_aliasdecl(const AliasDecl &n) {
  sync_to(n);
  dispatch(*n.value);
  sync_to(n.loc.end);
}

void Printer::visit_aliasrule(const AliasRule &n) {
  sync_to(n);
  if (!n.aliases.empty()) {
    sync_to(*n.aliases[0]);
    for (auto &a : n.aliases) {
      sync_to(*a);
      dispatch(*a);
    }
  }
  if (!n.rules.empty()) {
    sync_to(*n.rules[0]);
    for (auto &r : n.rules) {
      sync_to(*r);
      dispatch(*r);
    }
  }
  sync_to(n.loc.end);
}

void Printer::visit_aliasstmt(const AliasStmt &n) {
  sync_to(n);
  if (!n.aliases.empty()) {
    sync_to(*n.aliases[0]);
    for (auto &a : n.aliases) {
      sync_to(*a);
      dispatch(*a);
    }
  }
  if (!n.body.empty()) {
    sync_to(*n.body[0]);
    for (auto &s : n.body) {
      sync_to(*s);
      dispatch(*s);
    }
  }
  sync_to(n.loc.end);
}

void Printer::visit_and(const And &n) {
  visit_bexpr("and", n);
}

void Printer::visit_array(const Array &n) {
  sync_to(n);
  sync_to(*n.index_type);
  dispatch(*n.index_type);
  sync_to(*n.element_type);
  dispatch(*n.element_type);
  sync_to(n.loc.end);
}

void Printer::visit_assignment(const Assignment &n) {
  sync_to(n);
  dispatch(*n.lhs);
  sync_to(*n.rhs);
  dispatch(*n.rhs);
  sync_to(n.loc.end);
}

void Printer::visit_clear(const Clear &n) {
  sync_to(n);
  sync_to(*n.rhs);
  dispatch(*n.rhs);
  sync_to(n.loc.end);
}

void Printer::visit_constdecl(const ConstDecl &n) {
  sync_to(n);
  sync_to(*n.value);
  dispatch(*n.value);
  sync_to(n.loc.end);
}

void Printer::visit_div(const Div &n) {
  visit_bexpr("div", n);
}

void Printer::visit_element(const Element &n) {
  sync_to(n);
  sync_to(*n.array);
  dispatch(*n.array);
  sync_to(*n.index);
  dispatch(*n.index);
  sync_to(n.loc.end);
}

void Printer::visit_enum(const Enum &n) {
  sync_to(n);
  for (const std::pair<std::string, location> &m : n.members) {
    sync_to(m.second.begin);
    sync_to(m.second.end);
  }
  sync_to(n.loc.end);
}

void Printer::visit_eq(const Eq &n) {
  visit_bexpr("eq", n);
}

void Printer::visit_errorstmt(const ErrorStmt &n) {
  sync_to(n);
  sync_to(n.loc.end);
}

void Printer::visit_exists(const Exists &n) {
  sync_to(n);
  sync_to(n.quantifier);
  dispatch(n.quantifier);
  sync_to(*n.expr);
  dispatch(*n.expr);
  sync_to(n.loc.end);
}

void Printer::visit_exprid(const ExprID &n) {
  sync_to(n);
  sync_to(n.loc.end);
}

void Printer::visit_field(const Field &n) {
  sync_to(n);
  sync_to(*n.record);
  dispatch(*n.record);
  sync_to(n.loc.end);
}

void Printer::visit_for(const For &n) {
  sync_to(n);
  sync_to(n.quantifier);
  dispatch(n.quantifier);
  if (!n.body.empty()) {
    sync_to(*n.body[0]);
    for (auto &s : n.body) {
      sync_to(*s);
      dispatch(*s);
    }
  }
  sync_to(n.loc.end);
}

void Printer::visit_forall(const Forall &n) {
  sync_to(n);
  sync_to(n.quantifier);
  dispatch(n.quantifier);
  sync_to(*n.expr);
  dispatch(*n.expr);
  sync_to(n.loc.end);
}

void Printer::visit_function(const Function &n) {
  sync_to(n);
  if (!n.parameters.empty()) {
    sync_to(*n.parameters[0]);
    for (auto &p : n.parameters) {
      sync_to(*p);
      dispatch(*p);
    }
  }
  if (n.return_type != nullptr) {
    sync_to(*n.return_type);
    dispatch(*n.return_type);
  }
  if (!n.decls.empty()) {
    sync_to(*n.decls[0]);
    for (auto &d : n.decls) {
      sync_to(*d);
      dispatch(*d);
    }
  }
  if (!n.body.empty()) {
    sync_to(*n.body[0]);
    for (auto &s : n.body) {
      sync_to(*s);
      dispatch(*s);
    }
  }
  sync_to(n.loc.end);
}

void Printer::visit_functioncall(const FunctionCall &n) {
  sync_to(n);
  for (auto &a : n.arguments) {
    sync_to(*a);
    dispatch(*a);
  }
  sync_to(n.loc.end);
}

void Printer::visit_geq(const Geq &n) {
  visit_bexpr("geq", n);
}

void Printer::visit_gt(const Gt &n) {
  visit_bexpr("gt", n);
}

void Printer::visit_if(const If &n) {
  sync_to(n);
  for (const IfClause &c : n.clauses) {
    sync_to(c);
    dispatch(c);
  }
  sync_to(n.loc.end);
}

void Printer::visit_ifclause(const IfClause &n) {
  sync_to(n);
  if (n.condition != nullptr) {
    sync_to(*n.condition);
    dispatch(*n.condition);
  }
  if (!n.body.empty()) {
    sync_to(*n.body[0]);
    for (auto &s : n.body) {
      sync_to(*s);
      dispatch(*s);
    }
  }
  sync_to(n.loc.end);
}

void Printer::visit_implication(const Implication &n) {
  visit_bexpr("implication", n);
}

void Printer::visit_isundefined(const IsUndefined &n) {
  sync_to(n);
  sync_to(*n.expr);
  dispatch(*n.expr);
  sync_to(n.loc.end);
}

void Printer::visit_leq(const Leq &n) {
  visit_bexpr("leq", n);
}

void Printer::visit_lt(const Lt &n) {
  visit_bexpr("lt", n);
}

void Printer::visit_mod(const Mod &n) {
  visit_bexpr("mod", n);
}

void Printer::visit_model(const Model &n) {
  if (!n.decls.empty()) {
    sync_to(*n.decls[0]);
    for (auto &d : n.decls) {
      sync_to(*d);
      dispatch(*d);
    }
  }
  if (!n.functions.empty()) {
    sync_to(*n.functions[0]);
    for (auto f : n.functions) {
      sync_to(*f);
      dispatch(*f);
    }
  }
  if (!n.rules.empty()) {
    sync_to(*n.rules[0]);
    for (auto &r : n.rules) {
      sync_to(*r);
      dispatch(*r);
    }
  }
  sync_to(n.loc.end);
}

void Printer::visit_mul(const Mul &n) {
  visit_bexpr("mul", n);
}

void Printer::visit_negative(const Negative &n) {
  visit_uexpr("negative", n);
}

void Printer::visit_neq(const Neq &n) {
  visit_bexpr("neq", n);
}

void Printer::visit_not(const Not &n) {
  visit_uexpr("not", n);
}

void Printer::visit_number(const Number &n) {
  sync_to(n);
  sync_to(n.loc.end);
}

void Printer::visit_or(const Or &n) {
  visit_bexpr("or", n);
}

void Printer::visit_procedurecall(const ProcedureCall &n) {
  sync_to(n);
  sync_to(n.call);
  dispatch(n.call);
  sync_to(n.loc.end);
}

void Printer::visit_property(const Property &n) {
  sync_to(n);
  sync_to(*n.expr);
  dispatch(*n.expr);
  sync_to(n.loc.end);
}

void Printer::visit_propertyrule(const PropertyRule &n) {
  sync_to(n);
  if (!n.quantifiers.empty()) {
    sync_to(n.quantifiers[0]);
    for (const Quantifier &q : n.quantifiers) {
      sync_to(q);
      dispatch(q);
    }
  }
  sync_to(n.property);
  dispatch(n.property);
  sync_to(n.loc.end);
}

void Printer::visit_propertystmt(const PropertyStmt &n) {
  sync_to(n);
  sync_to(n.property);
  dispatch(n.property);
  sync_to(n.loc.end);
}

void Printer::visit_put(const Put &n) {
  sync_to(n);
  if (n.expr != nullptr) {
    sync_to(*n.expr);
    dispatch(*n.expr);
  }
  sync_to(n.loc.end);
}

void Printer::visit_quantifier(const Quantifier &n) {
  sync_to(n);
  if (n.type != nullptr) {
    sync_to(*n.type);
    dispatch(*n.type);
  }
  if (n.from != nullptr) {
    sync_to(*n.from);
    dispatch(*n.from);
  }
  if (n.to != nullptr) {
    sync_to(*n.to);
    dispatch(*n.to);
  }
  if (n.step != nullptr) {
    sync_to(*n.step);
    dispatch(*n.step);
  }
  sync_to(n.loc.end);
}

void Printer::visit_range(const Range &n) {
  sync_to(n);
  sync_to(*n.min);
  dispatch(*n.min);
  sync_to(*n.max);
  dispatch(*n.max);
  sync_to(n.loc.end);
}

void Printer::visit_record(const Record &n) {
  sync_to(n);
  for (auto &f : n.fields) {
    sync_to(*f);
    dispatch(*f);
  }
  sync_to(n.loc.end);
}

void Printer::visit_return(const Return &n) {
  sync_to(n);
  if (n.expr != nullptr) {
    sync_to(*n.expr);
    dispatch(*n.expr);
  }
  sync_to(n.loc.end);
}

void Printer::visit_ruleset(const Ruleset &n) {
  sync_to(n);
  if (!n.quantifiers.empty()) {
    sync_to(n.quantifiers[0]);
    for (const Quantifier &q : n.quantifiers) {
      sync_to(q);
      dispatch(q);
    }
  }
  if (!n.rules.empty()) {
    sync_to(*n.rules[0]);
    for (auto &r : n.rules) {
      sync_to(*r);
      dispatch(*r);
    }
  }
  sync_to(n.loc.end);
}

void Printer::visit_scalarset(const Scalarset &n) {
  sync_to(n);
  sync_to(*n.bound);
  dispatch(*n.bound);
  sync_to(n.loc.end);
}

void Printer::visit_simplerule(const SimpleRule &n) {
  sync_to(n);
  if (!n.quantifiers.empty()) {
    sync_to(n.quantifiers[0]);
    for (const Quantifier &q : n.quantifiers) {
      sync_to(q);
      dispatch(q);
    }
  }
  if (n.guard != nullptr) {
    sync_to(*n.guard);
    dispatch(*n.guard);
  }
  if (!n.decls.empty()) {
    sync_to(*n.decls[0]);
    for (auto &d : n.decls) {
      sync_to(*d);
      dispatch(*d);
    }
  }
  if (!n.body.empty()) {
    sync_to(*n.body[0]);
    for (auto &s : n.body) {
      sync_to(*s);
      dispatch(*s);
    }
  }
  sync_to(n.loc.end);
}

void Printer::visit_startstate(const StartState &n) {
  sync_to(n);
  if (!n.quantifiers.empty()) {
    sync_to(n.quantifiers[0]);
    for (const Quantifier &q : n.quantifiers) {
      sync_to(q);
      dispatch(q);
    }
  }
  if (!n.decls.empty()) {
    sync_to(*n.decls[0]);
    for (auto &d : n.decls) {
      sync_to(*d);
      dispatch(*d);
    }
  }
  if (!n.body.empty()) {
    sync_to(*n.body[0]);
    for (auto &s : n.body) {
      sync_to(*s);
      dispatch(*s);
    }
  }
  sync_to(n.loc.end);
}

void Printer::visit_sub(const Sub &n) {
  visit_bexpr("sub", n);
}

void Printer::visit_switch(const Switch &n) {
  sync_to(n);
  sync_to(*n.expr);
  dispatch(*n.expr);
  if (!n.cases.empty()) {
    sync_to(n.cases[0]);
    for (const SwitchCase &c : n.cases) {
      sync_to(c);
      dispatch(c);
    }
  }
  sync_to(n.loc.end);
}

void Printer::visit_switchcase(const SwitchCase &n) {
  sync_to(n);
  if (!n.matches.empty()) {
    sync_to(*n.matches[0]);
    for (auto &m : n.matches) {
      sync_to(*m);
      dispatch(*m);
    }
  }
  if (!n.body.empty()) {
    sync_to(*n.body[0]);
    for (auto &s : n.body) {
      sync_to(*s);
      dispatch(*s);
    }
  }
  sync_to(n.loc.end);
}

void Printer::visit_ternary(const Ternary &n) {
  sync_to(n);
  sync_to(*n.cond);
  dispatch(*n.cond);
  sync_to(*n.lhs);
  dispatch(*n.lhs);
  sync_to(*n.rhs);
  dispatch(*n.rhs);
  sync_to(n.loc.end);
}

void Printer::visit_typedecl(const TypeDecl &n) {
  sync_to(n);
  sync_to(*n.value);
  dispatch(*n.value);
  sync_to(n.loc.end);
}

void Printer::visit_typeexprid(const TypeExprID &n) {
  sync_to(n);
  sync_to(n.loc.end);
}

void Printer::visit_undefine(const Undefine &n) {
  sync_to(n);
  sync_to(*n.rhs);
  dispatch(*n.rhs);
  sync_to(n.loc.end);
}

void Printer::visit_vardecl(const VarDecl &n) {
  sync_to(n);
  sync_to(*n.type);
  dispatch(*n.type);
  sync_to(n.loc.end);
}

void Printer::visit_while(const While &n) {
  sync_to(n);
  sync_to(*n.condition);
  dispatch(*n.condition);
  if (!n.body.empty()) {
    sync_to(*n.body[0]);
    for (auto &s : n.body) {
      sync_to(*s);
      dispatch(*s);
    }
  }
  sync_to(n.loc.end);
}

Printer::~Printer() {
  sync_to();
  out.flush();
}

void Printer::sync_to(const Node &n) {
  sync_to(n.loc.begin);
}

void Printer::sync_to(const position &pos) {

  while (in.good() && (line < pos.line ||
         (line == pos.line && column < pos.column))) {

    int c = in.get();
    if (c == EOF) {
      break;
    }

    out << static_cast<char>(c);

    if (c == '\n') {
      line++;
      column = 1;
    } else {
      column++;
    }
  }
}

void Printer::visit_bexpr(const std::string &, const BinaryExpr &n) {
  sync_to(n);
  sync_to(*n.lhs);
  dispatch(*n.lhs);
  sync_to(*n.rhs);
  dispatch(*n.rhs);
  sync_to(n.loc.end);
}

void Printer::visit_uexpr(const std::string &, const UnaryExpr &n) {
  sync_to(n);
  sync_to(*n.rhs);
  dispatch(*n.rhs);
  sync_to(n.loc.end);
}
