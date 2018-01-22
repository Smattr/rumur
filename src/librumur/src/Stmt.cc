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

void Assert::validate() const { }

void Assert::generate(std::ostream &out) const {
  out << "if (__builtin_expect(!" << *expr << ", 0)) {\nthrow Error(\""
    << message << "\");\n}";
}

Assignment::Assignment(Lvalue *lhs_, Expr *rhs_, const location &loc_):
  Stmt(loc_), lhs(lhs_), rhs(rhs_) {
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

void Assignment::validate() const {
  lhs->validate();
  if (dynamic_cast<const SimpleTypeExpr*>(lhs->type()) == nullptr)
    throw RumurError("left hand side of assignment does not have a simple "
      "type", lhs->loc);
  rhs->validate();
}

void Assignment::generate(std::ostream &out) const {
  out << *lhs << " = " << *rhs;
}

Error::Error(const std::string &message_, const location &loc_):
  Stmt(loc_), message(message_) { }

Error::Error(const Error &other):
  Stmt(other.loc), message(other.message) { }

Error &Error::operator=(Error other) {
  swap(*this, other);
  return *this;
}

void swap(Error &x, Error &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.message, y.message);
}

Error *Error::clone() const {
  return new Error(*this);
}

void Error::validate() const { }

void Error::generate(std::ostream &out) const {
  out << "throw Error(\"" << message << "\")";
}

}
