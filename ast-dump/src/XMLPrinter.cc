#include <iostream>
#include <rumur/rumur.h>
#include <string>
#include "XMLPrinter.h"

using namespace rumur;

XMLPrinter::XMLPrinter(std::ostream &o_): o(&o_) { }

void XMLPrinter::visit(const Add &n) {
  visit_bexpr("add", n);
}

void XMLPrinter::visit(const And &n) {
  visit_bexpr("and", n);
}

void XMLPrinter::visit(const Array &n) {
  *o << "<array ";
  add_location(n);
  *o << "><indextype>";
  dispatch(*n.index_type);
  *o << "</indextype><elementtype>";
  dispatch(*n.element_type);
  *o << "</elementtype></array>";
}

void XMLPrinter::visit(const Assignment &n) {
  *o << "<assignment ";
  add_location(n);
  *o << "><lhs>";
  dispatch(*n.lhs);
  *o << "</lhs><rhs>";
  dispatch(*n.rhs);
  *o << "</rhs></assignment>";
}

void XMLPrinter::visit(const ConstDecl &n) {
  *o << "<constdecl name=\"" << n.name << "\" ";
  add_location(n);
  *o << "><value>";
  dispatch(*n.value);
  *o << "</value></constdecl>";
}

void XMLPrinter::visit(const Div &n) {
  visit_bexpr("div", n);
}

void XMLPrinter::visit(const Element &n) {
  *o << "<element ";
  add_location(n);
  *o << "><lhs>";
  dispatch(*n.array);
  *o << "</lhs><rhs>";
  dispatch(*n.index);
  *o << "</rhs></element>";
}

void XMLPrinter::visit(const Enum &n) {
  *o << "<enum ";
  add_location(n);
  *o << ">";
  for (const std::pair<std::string, location> &m : n.members)
    *o << "<member name=\"" << m.first << "\" "
       << "start_line=\"" << n.loc.begin.line << "\" "
       << "start_column=\"" << n.loc.begin.column << "\" "
       << "end_line=\"" << n.loc.end.line << "\" "
       << "end_column=\"" << n.loc.end.column << "\"/>";
  *o << "</enum>";
}

void XMLPrinter::visit(const Eq &n) {
  visit_bexpr("eq", n);
}

void XMLPrinter::visit(const ErrorStmt &n) {
  *o << "<errorstmt message=\"" << n.message << "\" ";
  add_location(n);
  *o << "/>";
}

void XMLPrinter::visit(const Exists &n) {
  *o << "<exists ";
  add_location(n);
  *o << "><quan>";
  dispatch(*n.quantifier);
  *o << "</quan><expr>";
  dispatch(*n.expr);
  *o << "</expr></exists>";
}

void XMLPrinter::visit(const ExprID &n) {
  *o << "<exprid id=\"" << n.id << "\" ";
  add_location(n);
  /* We deliberately omit printing n.value because this is the declaration we
   * discovered that this ID points to during symbol lookup. I.e. n.value is not
   * a "child" of this node in the sense of the source.
   */
  *o << "/>";
}

void XMLPrinter::visit(const Field &n) {
  *o << "<field ";
  add_location(n);
  *o << "><lhs>";
  dispatch(*n.record);
  *o << "</lhs><rhs><string>" << n.field << "</string></rhs></field>";
}

void XMLPrinter::visit(const For &n) {
  *o << "<forstmt ";
  add_location(n);
  *o << ">";
  dispatch(*n.quantifier);
  if (!n.body.empty()) {
    *o << "<body>";
    for (const Stmt *s : n.body)
      dispatch(*s);
    *o << "</body>";
  }
  *o << "</forstmt>";
}

void XMLPrinter::visit(const Forall &n) {
  *o << "<forall ";
  add_location(n);
  *o << "><quan>";
  dispatch(*n.quantifier);
  *o << "</quan><expr>";
  dispatch(*n.expr);
  *o << "</expr></forall>";
}

void XMLPrinter::visit(const Geq &n) {
  visit_bexpr("geq", n);
}

void XMLPrinter::visit(const Gt &n) {
  visit_bexpr("gt", n);
}

void XMLPrinter::visit(const If &n) {
  *o << "<if ";
  add_location(n);
  *o << ">";
  for (const IfClause &c : n.clauses)
    dispatch(c);
  *o << "</if>";
}

void XMLPrinter::visit(const IfClause &n) {
  *o << "<ifclause ";
  add_location(n);
  *o << ">";
  if (n.condition != nullptr) {
    *o << "<condition>";
    dispatch(*n.condition);
    *o << "</condition>";
  }
  if (!n.body.empty()) {
    *o << "<body>";
    for (const Stmt *s : n.body)
      dispatch(*s);
    *o << "</body>";
  }
  *o << "</ifclause>";
}

void XMLPrinter::visit(const Implication &n) {
  visit_bexpr("implication", n);
}

void XMLPrinter::visit(const Leq &n) {
  visit_bexpr("leq", n);
}

void XMLPrinter::visit(const Lt &n) {
  visit_bexpr("lt", n);
}

void XMLPrinter::visit(const Mod &n) {
  visit_bexpr("mod", n);
}

void XMLPrinter::visit(const Model &n) {
  *o << "<model ";
  add_location(n);
  *o << "><decls>";
  for (const Decl *d : n.decls)
    dispatch(*d);
  *o << "</decls><rules>";
  for (const Rule *r : n.rules)
    dispatch(*r);
  *o << "</rules></model>";
}

void XMLPrinter::visit(const Mul &n) {
  visit_bexpr("mul", n);
}

void XMLPrinter::visit(const Negative &n) {
  visit_uexpr("negative", n);
}

void XMLPrinter::visit(const Neq &n) {
  visit_bexpr("neq", n);
}

void XMLPrinter::visit(const Not &n) {
  visit_uexpr("not", n);
}

void XMLPrinter::visit(const Number &n) {
  *o << "<number value=\"" << n.value << "\" ";
  add_location(n);
  *o << "/>";
}

void XMLPrinter::visit(const Or &n) {
  visit_bexpr("or", n);
}

void XMLPrinter::visit(const Property &n) {
  *o << "<property category=\"";
  switch (n.category) {
    case Property::DISABLED:   *o << "disabled";   break;
    case Property::ASSERTION:  *o << "assertion";  break;
    case Property::ASSUMPTION: *o << "assumption"; break;
  }
  *o << "\" ";
  add_location(n);
  *o << "><expr>";
  dispatch(*n.expr);
  *o << "</expr></property>";
}

void XMLPrinter::visit(const PropertyRule &n) {
  *o << "<propertyrule name=\"" << n.name << "\" ";
  add_location(n);
  *o << ">";
  if (!n.quantifiers.empty()) {
    *o << "<quantifiers>";
    for (const Quantifier *q : n.quantifiers)
      dispatch(*q);
    *o << "</quantifiers>";
  }
  dispatch(n.property);
  *o << "</propertyrule>";
}

void XMLPrinter::visit(const PropertyStmt &n) {
  *o << "<propertystmt message=\"" << n.message << "\" ";
  add_location(n);
  *o << ">";
  dispatch(n.property);
  *o << "</propertystmt>";
}

void XMLPrinter::visit(const Quantifier &n) {
  *o << "<quantifier ";
  add_location(n);
  *o << "><var>";
  dispatch(*n.var);
  *o << "</var>";
  if (n.step != nullptr) {
    *o << "<step>";
    dispatch(*n.step);
    *o << "</step>";
  }
  *o << "</quantifier>";
}

void XMLPrinter::visit(const Range &n) {
  *o << "<range ";
  add_location(n);
  *o << "><min>";
  dispatch(*n.min);
  *o << "</min><max>";
  dispatch(*n.max);
  *o << "</max></range>";
}

void XMLPrinter::visit(const Record &n) {
  *o << "<record ";
  add_location(n);
  *o << ">";
  for (const VarDecl *f : n.fields)
    dispatch(*f);
  *o << "</record>";
}

void XMLPrinter::visit(const Return &n) {
  *o << "<return ";
  add_location(n);
  *o << ">";
  if (n.expr != nullptr)
    dispatch(*n.expr);
  *o << "</return>";
}

void XMLPrinter::visit(const Ruleset &n) {
  *o << "<ruleset name=\"" << n.name << "\" ";
  add_location(n);
  *o << ">";
  if (!n.quantifiers.empty()) {
    *o << "<quantifiers>";
    for (const Quantifier *q : n.quantifiers)
      dispatch(*q);
    *o << "</quantifiers>";
  }
  if (!n.rules.empty()) {
    *o << "<rules>";
    for (const Rule *r : n.rules)
      dispatch(*r);
    *o << "</rules>";
  }
  *o << "</ruleset>";
}

void XMLPrinter::visit(const Scalarset &n) {
  *o << "<scalarset ";
  add_location(n);
  *o << "><bound>";
  dispatch(*n.bound);
  *o << "</bound></scalarset>";
}

void XMLPrinter::visit(const SimpleRule &n) {
  *o << "<simplerule name=\"" << n.name << "\" ";
  add_location(n);
  *o << ">";
  if (!n.quantifiers.empty()) {
    *o << "<quantifiers>";
    for (const Quantifier *q : n.quantifiers)
      dispatch(*q);
    *o << "</quantifiers>";
  }
  if (n.guard != nullptr) {
    *o << "<guard>";
    dispatch(*n.guard);
    *o << "</guard>";
  }
  if (!n.decls.empty()) {
    *o << "<decls>";
    for (const Decl *d : n.decls)
      dispatch(*d);
    *o << "</decls>";
  }
  if (!n.body.empty()) {
    *o << "<body>";
    for (const Stmt *s : n.body)
      dispatch(*s);
    *o << "</body>";
  }
  *o << "</simplerule>";
}

void XMLPrinter::visit(const StartState &n) {
  *o << "<startstate name=\"" << n.name << "\" ";
  add_location(n);
  *o << ">";
  if (!n.quantifiers.empty()) {
    *o << "<quantifiers>";
    for (const Quantifier *q : n.quantifiers)
      dispatch(*q);
    *o << "</quantifiers>";
  }
  if (!n.decls.empty()) {
    *o << "<decls>";
    for (const Decl *d : n.decls)
      dispatch(*d);
    *o << "</decls>";
  }
  if (!n.body.empty()) {
    *o << "<body>";
    for (const Stmt *s : n.body)
      dispatch(*s);
    *o << "</body>";
  }
  *o << "</startstate>";
}

void XMLPrinter::visit(const Sub &n) {
  visit_bexpr("sub", n);
}

void XMLPrinter::visit(const Ternary &n) {
  *o << "<ternary ";
  add_location(n);
  *o << "><condition>";
  dispatch(*n.cond);
  *o << "</condition><lhs>";
  dispatch(*n.lhs);
  *o << "</lhs><rhs>";
  dispatch(*n.rhs);
  *o << "</rhs></ternary>";
}

void XMLPrinter::visit(const TypeDecl &n) {
  *o << "<typedecl name=\"" << n.name << "\" ";
  add_location(n);
  *o << "><value>";
  dispatch(*n.value);
  *o << "</value></typedecl>";
}

void XMLPrinter::visit(const TypeExprID &n) {
  *o << "<typeexprid name=\"" << n.name << "\" ";
  add_location(n);
  *o << "/>";
  /* We deliberately omit n.referent because this is a result of symbol
   * resolution and not morally a child of this node.
   */
}

void XMLPrinter::visit(const Undefine &n) {
  *o << "<undefine ";
  add_location(n);
  *o << ">";
  dispatch(*n.rhs);
  *o << "</undefine>";
}

void XMLPrinter::visit(const VarDecl &n) {
  *o << "<vardecl name=\"" << n.name << "\" ";
  add_location(n);
  *o << "><type>";
  dispatch(*n.type);
  *o << "</type></vardecl>";
}

XMLPrinter::~XMLPrinter() {
  o->flush();
}

void XMLPrinter::add_location(const Node &n) {
  *o << "start_line=\"" << n.loc.begin.line <<
     "\" start_column=\"" << n.loc.begin.column <<
     "\" end_line=\"" << n.loc.end.line <<
     "\" end_column=\"" << n.loc.end.column << "\"";
}

void XMLPrinter::visit_bexpr(const std::string &tag, const BinaryExpr &n) {
  *o << "<" << tag << " ";
  add_location(n);
  *o << "><lhs>";
  dispatch(*n.lhs);
  *o << "</lhs><rhs>";
  dispatch(*n.rhs);
  *o << "</rhs></" << tag << ">";
}

void XMLPrinter::visit_uexpr(const std::string &tag, const UnaryExpr &n) {
  *o << "<" << tag << " ";
  add_location(n);
  *o << "><rhs>";
  dispatch(*n.rhs);
  *o << "</rhs></" << tag << ">";
}
