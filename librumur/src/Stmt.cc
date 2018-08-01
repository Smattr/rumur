#include <cassert>
#include <iostream>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Property.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>

namespace rumur {

PropertyStmt::PropertyStmt(const Property &property_,
  const std::string &message_, const location &loc_):
  Stmt(loc_), property(property_), message(message_) { }

PropertyStmt::PropertyStmt(const PropertyStmt &other):
  Stmt(other.loc), property(other.property), message(other.message) { }

PropertyStmt &PropertyStmt::operator=(PropertyStmt other) {
  swap(*this, other);
  return *this;
}

void swap(PropertyStmt &x, PropertyStmt &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.property, y.property);
  swap(x.message, y.message);
}

PropertyStmt *PropertyStmt::clone() const {
  return new PropertyStmt(*this);
}

void PropertyStmt::generate(std::ostream &out) const {
  switch (property.category) {

    case Property::ASSERTION:
      out << "if (__builtin_expect(!";
      property.generate(out);
      out << ", 0)) {\nerror(s, \"" << message << "\");\n}";
      break;

    default:
      assert(!"unimplemented");

  }
}

bool PropertyStmt::operator==(const Node &other) const {
  auto o = dynamic_cast<const PropertyStmt*>(&other);
  return o != nullptr && property == o->property && message == o->message;
}

Assignment::Assignment(Lvalue *lhs_, Expr *rhs_, const location &loc_):
  Stmt(loc_), lhs(lhs_), rhs(rhs_) {
  if (!lhs->type()->is_simple())
    throw Error("left hand side of assignment does not have a simple "
      "type", lhs->loc);
}

Assignment::Assignment(const Assignment &other):
  Stmt(other), lhs(other.lhs->clone()), rhs(other.rhs->clone()) {
}

Assignment &Assignment::operator=(Assignment other) {
  swap(*this, other);
  return *this;
}

void swap(Assignment &x, Assignment &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.lhs, y.lhs);
  swap(x.rhs, y.rhs);
}

Assignment *Assignment::clone() const {
  return new Assignment(*this);
}

Assignment::~Assignment() {
  delete lhs;
  delete rhs;
}

void Assignment::generate(std::ostream &out) const {

  const std::string lb = lhs->type()->lower_bound();
  const std::string ub = lhs->type()->upper_bound();

  out << "handle_write(s, " << lb << ", " << ub << ", ";
  lhs->generate_lvalue(out);
  out << ", " << *rhs << ")";
}

bool Assignment::operator==(const Node &other) const {
  auto o = dynamic_cast<const Assignment*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

ErrorStmt::ErrorStmt(const std::string &message_, const location &loc_):
  Stmt(loc_), message(message_) { }

ErrorStmt::ErrorStmt(const ErrorStmt &other):
  Stmt(other.loc), message(other.message) { }

ErrorStmt &ErrorStmt::operator=(ErrorStmt other) {
  swap(*this, other);
  return *this;
}

void swap(ErrorStmt &x, ErrorStmt &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.message, y.message);
}

ErrorStmt *ErrorStmt::clone() const {
  return new ErrorStmt(*this);
}

void ErrorStmt::generate(std::ostream &out) const {
  out << "throw Error(\"" << message << "\")";
}

bool ErrorStmt::operator==(const Node &other) const {
  auto o = dynamic_cast<const ErrorStmt*>(&other);
  return o != nullptr && message == o->message;
}

For::For(Quantifier *quantifier_, std::vector<Stmt*> &&body_,
  const location &loc_):
  Stmt(loc_), quantifier(quantifier_), body(body_) { }

For::For(const For &other):
  Stmt(other.loc), quantifier(other.quantifier->clone()) {
  for (const Stmt *s : other.body)
    body.push_back(s->clone());
}

For &For::operator=(For other) {
  swap(*this, other);
  return *this;
}

void swap(For &x, For &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.quantifier, y.quantifier);
  swap(x.body, y.body);
}

For *For::clone() const {
  return new For(*this);
}

void For::generate(std::ostream &out) const {
  quantifier->generate_header(out);
  for (Stmt const *s : body) {
    out << "  ";
    s->generate(out);
    out << ";\n";
  }
  quantifier->generate_footer(out);
}

bool For::operator==(const Node &other) const {
  auto o = dynamic_cast<const For*>(&other);
  if (o == nullptr)
    return false;
  if (*quantifier != *o->quantifier)
    return false;
  for (auto it = body.begin(), it2 = o->body.begin(); ; it++, it2++) {
    if (it == body.end()) {
      if (it2 != o->body.end())
        return false;
      break;
    }
    if (it2 == o->body.end())
      return false;
    if (**it != **it2)
      return false;
  }
  return true;
}

For::~For() {
  delete quantifier;
  for (Stmt *s : body)
    delete s;
}

IfClause::IfClause(Expr *condition_, std::vector<Stmt*> &&body_,
  const location &loc_):
  Node(loc_), condition(condition_), body(body_) { }

IfClause::IfClause(const IfClause &other):
  Node(other.loc),
  condition(other.condition == nullptr ? nullptr : other.condition->clone()) {
  for (const Stmt *s : other.body)
    body.push_back(s->clone());
}

IfClause &IfClause::operator=(IfClause other) {
  swap(*this, other);
  return *this;
}

void swap(IfClause &x, IfClause &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.condition, y.condition);
  swap(x.body, y.body);
}

IfClause::~IfClause() {
  delete condition;
  for (Stmt *s : body)
    delete s;
}

IfClause *IfClause::clone() const {
  return new IfClause(*this);
}

bool IfClause::operator==(const Node &other) const {
  auto o = dynamic_cast<const IfClause*>(&other);
  if (o == nullptr)
    return false;
  if (condition == nullptr) {
    if (o->condition != nullptr)
      return false;
  } else if (o->condition == nullptr) {
    return false;
  } else if (*condition != *o->condition) {
    return false;
  }
  for (auto it = body.begin(), it2 = o->body.begin(); ; it++, it2++) {
    if (it == body.end()) {
      if (it2 != o->body.end())
        return false;
      break;
    }
    if (it2 == o->body.end())
      return false;
    if (**it != **it2)
      return false;
  }
  return true;
}

If::If(std::vector<IfClause> &&clauses_, const location &loc_):
  Stmt(loc_), clauses(clauses_) { }

If::If(const If &other):
  Stmt(other.loc), clauses(other.clauses) { }

If &If::operator=(If other) {
  swap(*this, other);
  return *this;
}

void swap(If &x, If &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.clauses, y.clauses);
}

If *If::clone() const {
  return new If(*this);
}

void If::generate(std::ostream &out) const {
  bool first = true;
  for (const IfClause &c : clauses) {
    if (!first)
      out << "else ";
    if (c.condition != nullptr) {
      out << "if (";
      c.condition->generate_rvalue(out);
      out  << ") ";
    }
    out << " {\n";
    for (const Stmt *s : c.body) {
      s->generate(out);
      out << ";\n";
    }
    out << "}\n";
    first = false;
  }
}

bool If::operator==(const Node &other) const {
  auto o = dynamic_cast<const If*>(&other);
  if (o == nullptr)
    return false;
  if (clauses != o->clauses)
    return false;
  return true;
}

Return::Return(Expr *expr_, const location &loc_):
  Stmt(loc_), expr(expr_) { }

Return::Return(const Return &other):
  Stmt(other.loc), expr(other.expr == nullptr ? nullptr : other.expr->clone()) { }

Return &Return::operator=(Return other) {
  swap(*this, other);
  return *this;
}

void swap(Return &x, Return &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.expr, y.expr);
}

Return::~Return() {
  delete expr;
}

Return *Return::clone() const {
  return new Return(*this);
}

void Return::generate(std::ostream &out) const {
  out << "return";

  if (expr != nullptr)
    assert(!"TODO");
}

bool Return::operator==(const Node &other) const {
  auto o = dynamic_cast<const Return*>(&other);
  if (o == nullptr)
    return false;
  if (expr == nullptr) {
    if (o->expr != nullptr)
      return false;
  } else if (o->expr == nullptr) {
    return false;
  } else if (*expr != *o->expr) {
    return false;
  }
  return true;
}

Undefine::Undefine(Lvalue *rhs_, const location &loc_):
  Stmt(loc_), rhs(rhs_) { }

Undefine::Undefine(const Undefine &other):
  Stmt(other.loc), rhs(other.rhs->clone()) { }

Undefine &Undefine::operator=(Undefine other) {
  swap(*this, other);
  return *this;
}

void swap(Undefine &x, Undefine &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.rhs, y.rhs);
}

Undefine::~Undefine() {
  delete rhs;
}

Undefine *Undefine::clone() const {
  return new Undefine(*this);
}

void Undefine::generate(std::ostream &out) const {
  out << "handle_zero(";
  rhs->generate_lvalue(out);
  out << ")";
}

bool Undefine::operator==(const Node &other) const {
  auto o = dynamic_cast<const Undefine*>(&other);
  if (o == nullptr)
    return false;
  if (*rhs != *o->rhs)
    return false;
  return true;
}

}
