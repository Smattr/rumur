#include "Printer.h"
#include <cassert>
#include <cstddef>
#include <iostream>
#include <rumur/rumur.h>
#include <sstream>
#include <string>

using namespace rumur;

Printer::Printer(std::istream &in_, std::ostream &out_) : in(in_), out(out_) {}

void Printer::visit_add(const Add &n) { visit_bexpr(n); }

void Printer::visit_aliasdecl(const AliasDecl &n) {
  top->sync_to(n);
  top->dispatch(*n.value);
  top->sync_to(n.loc.end);
}

void Printer::visit_aliasrule(const AliasRule &n) {
  top->sync_to(n);
  if (!n.aliases.empty()) {
    top->sync_to(*n.aliases[0]);
    for (auto &a : n.aliases) {
      top->sync_to(*a);
      top->dispatch(*a);
    }
  }
  if (!n.rules.empty()) {
    top->sync_to(*n.rules[0]);
    for (auto &r : n.rules) {
      top->sync_to(*r);
      top->dispatch(*r);
    }
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_aliasstmt(const AliasStmt &n) {
  top->sync_to(n);
  if (!n.aliases.empty()) {
    top->sync_to(*n.aliases[0]);
    for (auto &a : n.aliases) {
      top->sync_to(*a);
      top->dispatch(*a);
    }
  }
  if (!n.body.empty()) {
    top->sync_to(*n.body[0]);
    for (auto &s : n.body) {
      top->sync_to(*s);
      top->dispatch(*s);
    }
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_and(const And &n) { visit_bexpr(n); }

void Printer::visit_array(const Array &n) {
  top->sync_to(n);
  top->sync_to(*n.index_type);
  top->dispatch(*n.index_type);
  top->sync_to(*n.element_type);
  top->dispatch(*n.element_type);
  top->sync_to(n.loc.end);
}

void Printer::visit_band(const Band &n) { visit_bexpr(n); }

void Printer::visit_bnot(const Bnot &n) { visit_uexpr(n); }

void Printer::visit_bor(const Bor &n) { visit_bexpr(n); }

void Printer::visit_assignment(const Assignment &n) {
  top->sync_to(n);
  top->dispatch(*n.lhs);
  top->sync_to(*n.rhs);
  top->dispatch(*n.rhs);
  top->sync_to(n.loc.end);
}

void Printer::visit_clear(const Clear &n) {
  top->sync_to(n);
  top->sync_to(*n.rhs);
  top->dispatch(*n.rhs);
  top->sync_to(n.loc.end);
}

void Printer::visit_constdecl(const ConstDecl &n) {
  top->sync_to(n);
  top->sync_to(*n.value);
  top->dispatch(*n.value);
  top->sync_to(n.loc.end);
}

void Printer::visit_div(const Div &n) { visit_bexpr(n); }

void Printer::visit_element(const Element &n) {
  top->sync_to(n);
  top->sync_to(*n.array);
  top->dispatch(*n.array);
  top->sync_to(*n.index);
  top->dispatch(*n.index);
  top->sync_to(n.loc.end);
}

void Printer::visit_enum(const Enum &n) {
  top->sync_to(n);
  for (const std::pair<std::string, location> &m : n.members) {
    top->sync_to(m.second.begin);
    top->sync_to(m.second.end);
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_eq(const Eq &n) { visit_bexpr(n); }

void Printer::visit_errorstmt(const ErrorStmt &n) {
  top->sync_to(n);
  top->sync_to(n.loc.end);
}

void Printer::visit_exists(const Exists &n) {
  top->sync_to(n);
  top->sync_to(n.quantifier);
  top->dispatch(n.quantifier);
  top->sync_to(*n.expr);
  top->dispatch(*n.expr);
  top->sync_to(n.loc.end);
}

void Printer::visit_exprid(const ExprID &n) {
  top->sync_to(n);
  top->sync_to(n.loc.end);
}

void Printer::visit_field(const Field &n) {
  top->sync_to(n);
  top->sync_to(*n.record);
  top->dispatch(*n.record);
  top->sync_to(n.loc.end);
}

void Printer::visit_for(const For &n) {
  top->sync_to(n);
  top->sync_to(n.quantifier);
  top->dispatch(n.quantifier);
  if (!n.body.empty()) {
    top->sync_to(*n.body[0]);
    for (auto &s : n.body) {
      top->sync_to(*s);
      top->dispatch(*s);
    }
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_forall(const Forall &n) {
  top->sync_to(n);
  top->sync_to(n.quantifier);
  top->dispatch(n.quantifier);
  top->sync_to(*n.expr);
  top->dispatch(*n.expr);
  top->sync_to(n.loc.end);
}

void Printer::visit_function(const Function &n) {
  top->sync_to(n);
  if (!n.parameters.empty()) {
    top->sync_to(*n.parameters[0]);
    for (auto &p : n.parameters) {
      top->sync_to(*p);
      top->dispatch(*p);
    }
  }
  if (n.return_type != nullptr) {
    top->sync_to(*n.return_type);
    top->dispatch(*n.return_type);
  }
  if (!n.decls.empty()) {
    top->sync_to(*n.decls[0]);
    for (auto &d : n.decls) {
      top->sync_to(*d);
      top->dispatch(*d);
    }
  }
  if (!n.body.empty()) {
    top->sync_to(*n.body[0]);
    for (auto &s : n.body) {
      top->sync_to(*s);
      top->dispatch(*s);
    }
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_functioncall(const FunctionCall &n) {
  top->sync_to(n);
  for (auto &a : n.arguments) {
    top->sync_to(*a);
    top->dispatch(*a);
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_geq(const Geq &n) { visit_bexpr(n); }

void Printer::visit_gt(const Gt &n) { visit_bexpr(n); }

void Printer::visit_if(const If &n) {
  top->sync_to(n);
  for (const IfClause &c : n.clauses) {
    top->sync_to(c);
    top->dispatch(c);
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_ifclause(const IfClause &n) {
  top->sync_to(n);
  if (n.condition != nullptr) {
    top->sync_to(*n.condition);
    top->dispatch(*n.condition);
  }
  if (!n.body.empty()) {
    top->sync_to(*n.body[0]);
    for (auto &s : n.body) {
      top->sync_to(*s);
      top->dispatch(*s);
    }
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_implication(const Implication &n) { visit_bexpr(n); }

void Printer::visit_isundefined(const IsUndefined &n) { visit_uexpr(n); }

void Printer::visit_leq(const Leq &n) { visit_bexpr(n); }

void Printer::visit_lsh(const Lsh &n) { visit_bexpr(n); }

void Printer::visit_lt(const Lt &n) { visit_bexpr(n); }

void Printer::visit_mod(const Mod &n) { visit_bexpr(n); }

void Printer::visit_model(const Model &n) {
  if (!n.children.empty()) {
    top->sync_to(*n.children[0]);
    for (const Ptr<Node> &c : n.children) {
      top->sync_to(*c);
      top->dispatch(*c);
    }
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_mul(const Mul &n) { visit_bexpr(n); }

void Printer::visit_negative(const Negative &n) { visit_uexpr(n); }

void Printer::visit_neq(const Neq &n) { visit_bexpr(n); }

void Printer::visit_not(const Not &n) { visit_uexpr(n); }

void Printer::visit_number(const Number &n) {
  top->sync_to(n);
  top->sync_to(n.loc.end);
}

void Printer::visit_or(const Or &n) { visit_bexpr(n); }

void Printer::visit_procedurecall(const ProcedureCall &n) {
  top->sync_to(n);
  top->sync_to(n.call);
  top->dispatch(n.call);
  top->sync_to(n.loc.end);
}

void Printer::visit_property(const Property &n) {
  top->sync_to(n);
  top->sync_to(*n.expr);
  top->dispatch(*n.expr);
  top->sync_to(n.loc.end);
}

void Printer::visit_propertyrule(const PropertyRule &n) {
  top->sync_to(n);
  if (!n.quantifiers.empty()) {
    top->sync_to(n.quantifiers[0]);
    for (const Quantifier &q : n.quantifiers) {
      top->sync_to(q);
      top->dispatch(q);
    }
  }
  top->sync_to(n.property);
  top->dispatch(n.property);
  top->sync_to(n.loc.end);
}

void Printer::visit_propertystmt(const PropertyStmt &n) {
  top->sync_to(n);
  top->sync_to(n.property);
  top->dispatch(n.property);
  top->sync_to(n.loc.end);
}

void Printer::visit_put(const Put &n) {
  top->sync_to(n);
  if (n.expr != nullptr) {
    top->sync_to(*n.expr);
    top->dispatch(*n.expr);
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_quantifier(const Quantifier &n) {
  top->sync_to(n);
  if (n.type != nullptr) {
    top->sync_to(*n.type);
    top->dispatch(*n.type);
  }
  if (n.from != nullptr) {
    top->sync_to(*n.from);
    top->dispatch(*n.from);
  }
  if (n.to != nullptr) {
    top->sync_to(*n.to);
    top->dispatch(*n.to);
  }
  if (n.step != nullptr) {
    top->sync_to(*n.step);
    top->dispatch(*n.step);
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_range(const Range &n) {
  top->sync_to(n);
  top->sync_to(*n.min);
  top->dispatch(*n.min);
  top->sync_to(*n.max);
  top->dispatch(*n.max);
  top->sync_to(n.loc.end);
}

void Printer::visit_record(const Record &n) {
  top->sync_to(n);
  for (auto &f : n.fields) {
    top->sync_to(*f);
    top->dispatch(*f);
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_return(const Return &n) {
  top->sync_to(n);
  if (n.expr != nullptr) {
    top->sync_to(*n.expr);
    top->dispatch(*n.expr);
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_rsh(const Rsh &n) { visit_bexpr(n); }

void Printer::visit_ruleset(const Ruleset &n) {
  top->sync_to(n);
  if (!n.quantifiers.empty()) {
    top->sync_to(n.quantifiers[0]);
    for (const Quantifier &q : n.quantifiers) {
      top->sync_to(q);
      top->dispatch(q);
    }
  }
  if (!n.rules.empty()) {
    top->sync_to(*n.rules[0]);
    for (auto &r : n.rules) {
      top->sync_to(*r);
      top->dispatch(*r);
    }
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_scalarset(const Scalarset &n) {
  top->sync_to(n);
  top->sync_to(*n.bound);
  top->dispatch(*n.bound);
  top->sync_to(n.loc.end);
}

void Printer::visit_simplerule(const SimpleRule &n) {
  top->sync_to(n);
  if (!n.quantifiers.empty()) {
    top->sync_to(n.quantifiers[0]);
    for (const Quantifier &q : n.quantifiers) {
      top->sync_to(q);
      top->dispatch(q);
    }
  }
  if (n.guard != nullptr) {
    top->sync_to(*n.guard);
    top->dispatch(*n.guard);
  }
  if (!n.decls.empty()) {
    top->sync_to(*n.decls[0]);
    for (auto &d : n.decls) {
      top->sync_to(*d);
      top->dispatch(*d);
    }
  }
  if (!n.body.empty()) {
    top->sync_to(*n.body[0]);
    for (auto &s : n.body) {
      top->sync_to(*s);
      top->dispatch(*s);
    }
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_startstate(const StartState &n) {
  top->sync_to(n);
  if (!n.quantifiers.empty()) {
    top->sync_to(n.quantifiers[0]);
    for (const Quantifier &q : n.quantifiers) {
      top->sync_to(q);
      top->dispatch(q);
    }
  }
  if (!n.decls.empty()) {
    top->sync_to(*n.decls[0]);
    for (auto &d : n.decls) {
      top->sync_to(*d);
      top->dispatch(*d);
    }
  }
  if (!n.body.empty()) {
    top->sync_to(*n.body[0]);
    for (auto &s : n.body) {
      top->sync_to(*s);
      top->dispatch(*s);
    }
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_sub(const Sub &n) { visit_bexpr(n); }

void Printer::visit_switch(const Switch &n) {
  top->sync_to(n);
  top->sync_to(*n.expr);
  top->dispatch(*n.expr);
  if (!n.cases.empty()) {
    top->sync_to(n.cases[0]);
    for (const SwitchCase &c : n.cases) {
      top->sync_to(c);
      top->dispatch(c);
    }
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_switchcase(const SwitchCase &n) {
  top->sync_to(n);
  if (!n.matches.empty()) {
    top->sync_to(*n.matches[0]);
    for (auto &m : n.matches) {
      top->sync_to(*m);
      top->dispatch(*m);
    }
  }
  if (!n.body.empty()) {
    top->sync_to(*n.body[0]);
    for (auto &s : n.body) {
      top->sync_to(*s);
      top->dispatch(*s);
    }
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_ternary(const Ternary &n) {
  top->sync_to(n);
  top->sync_to(*n.cond);
  top->dispatch(*n.cond);
  top->sync_to(*n.lhs);
  top->dispatch(*n.lhs);
  top->sync_to(*n.rhs);
  top->dispatch(*n.rhs);
  top->sync_to(n.loc.end);
}

void Printer::visit_typedecl(const TypeDecl &n) {
  top->sync_to(n);
  top->sync_to(*n.value);
  top->dispatch(*n.value);
  top->sync_to(n.loc.end);
}

void Printer::visit_typeexprid(const TypeExprID &n) {
  top->sync_to(n);
  top->sync_to(n.loc.end);
}

void Printer::visit_undefine(const Undefine &n) {
  top->sync_to(n);
  top->sync_to(*n.rhs);
  top->dispatch(*n.rhs);
  top->sync_to(n.loc.end);
}

void Printer::visit_vardecl(const VarDecl &n) {
  top->sync_to(n);
  top->sync_to(*n.type);
  top->dispatch(*n.type);
  top->sync_to(n.loc.end);
}

void Printer::visit_while(const While &n) {
  top->sync_to(n);
  top->sync_to(*n.condition);
  top->dispatch(*n.condition);
  if (!n.body.empty()) {
    top->sync_to(*n.body[0]);
    for (auto &s : n.body) {
      top->sync_to(*s);
      top->dispatch(*s);
    }
  }
  top->sync_to(n.loc.end);
}

void Printer::visit_xor(const Xor &n) { visit_bexpr(n); }

void Printer::process(const Token &t) {

  assert(t.type != Token::SUBJ &&
         "an IntermediateStage did not put a shift "
         "message in the processing pipe that they did not consume");

  out << t.character;
}

void Printer::sync_to(const Node &n) { sync_to(n.loc.begin); }

void Printer::sync_to(const position &pos) {

  // the type of position.line and position.column changes across Bison
  // releases, so avoid some -Wsign-compare warnings by casting them in advance
  auto pos_line = static_cast<unsigned long>(pos.line);
  auto pos_col = static_cast<unsigned long>(pos.column);

  // buffer up to the given position
  std::ostringstream buffer;
  while (in.good() &&
         (line < pos_line || (line == pos_line && column < pos_col))) {

    int c = in.get();
    if (c == EOF)
      break;

    buffer << static_cast<char>(c);

    if (c == '\n') {
      line++;
      column = 1;
    } else {
      column++;
    }
  }

  // if we actually advanced in the input, output the data
  if (buffer.str() != "")
    *top << buffer.str();
}

void Printer::skip_to(const Node &n) { skip_to(n.loc.begin); }

void Printer::skip_to(const position &pos) {

  // the type of position.line and position.column changes across Bison
  // releases, so avoid some -Wsign-compare warnings by casting them in advance
  auto pos_line = static_cast<unsigned long>(pos.line);
  auto pos_col = static_cast<unsigned long>(pos.column);

  // if this is ahead of our current position, seek forwards while dropping
  // characters
  while (in.good() &&
         (line < pos_line || (line == pos_line && column < pos_col))) {

    int c = in.get();
    if (c == EOF)
      break;

    if (c == '\n') {
      line++;
      column = 1;
    } else {
      column++;
    }
  }
}

void Printer::finalise() { out.flush(); }

void Printer::visit_bexpr(const BinaryExpr &n) {
  top->sync_to(n);
  top->sync_to(*n.lhs);
  top->dispatch(*n.lhs);
  top->sync_to(*n.rhs);
  top->dispatch(*n.rhs);
  top->sync_to(n.loc.end);
}

void Printer::visit_uexpr(const UnaryExpr &n) {
  top->sync_to(n);
  top->sync_to(*n.rhs);
  top->dispatch(*n.rhs);
  top->sync_to(n.loc.end);
}
