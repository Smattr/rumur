#include <cassert>
#include <iostream>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>

namespace rumur {

Assert::Assert(Expr *expr_, const std::string &message_, const location &loc_):
  Stmt(loc_), expr(expr_), message(message_) { }

Assert::Assert(const Assert &other):
  Stmt(other.loc), expr(other.expr), message(other.message) { }

Assert &Assert::operator=(Assert other) {
  swap(*this, other);
  return *this;
}

void swap(Assert &x, Assert &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.expr, y.expr);
  swap(x.message, y.message);
}

Assert *Assert::clone() const {
  return new Assert(*this);
}

Assert::~Assert() {
  delete expr;
}

void Assert::generate(std::ostream &out) const {
  out << "if (__builtin_expect(!" << *expr << ", 0)) {\nthrow Error(\""
    << message << "\");\n}";
}

bool Assert::operator==(const Node &other) const {
  auto o = dynamic_cast<const Assert*>(&other);
  return o != nullptr && *expr == *o->expr && message == o->message;
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
  out << "handle_write(";
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

}
