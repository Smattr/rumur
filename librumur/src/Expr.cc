#include <cassert>
#include <cstdint>
#include <gmpxx.h>
#include <iostream>
#include <limits>
#include "location.hh"
#include <memory>
#include <rumur/Boolean.h>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Function.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>
#include "vector_utils.h"

namespace rumur {

bool Expr::is_boolean() const {
  return type() != nullptr && *type()->resolve() == *Boolean;
}

Ternary::Ternary(std::shared_ptr<Expr> cond_, std::shared_ptr<Expr> lhs_,
  std::shared_ptr<Expr> rhs_, const location &loc_):
  Expr(loc_), cond(cond_), lhs(lhs_), rhs(rhs_) {
  validate();
}

Ternary::Ternary(const Ternary &other):
  Expr(other), cond(other.cond->clone()), lhs(other.lhs->clone()),
  rhs(other.rhs->clone()) {
}

Ternary &Ternary::operator=(Ternary other) {
  swap(*this, other);
  return *this;
}

void swap(Ternary &x, Ternary &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.cond, y.cond);
  swap(x.lhs, y.lhs);
  swap(x.rhs, y.rhs);
}

Ternary *Ternary::clone() const {
  return new Ternary(*this);
}

bool Ternary::constant() const {
  return cond->constant() && lhs->constant() && rhs->constant();
}

bool Ternary::operator==(const Node &other) const {
  auto o = dynamic_cast<const Ternary*>(&other);
  return o != nullptr && *cond == *o->cond && *lhs == *o->lhs && *rhs == *o->rhs;
}

const TypeExpr *Ternary::type() const {
  // TODO: assert lhs and rhs are compatible types.
  return lhs->type();
}

void Ternary::generate_rvalue(std::ostream &out) const {
  out << "(" << *cond << " ? " << *lhs << " : " << *rhs << ")";
}

mpz_class Ternary::constant_fold() const {
  return cond->constant_fold() ? lhs->constant_fold() : rhs->constant_fold();
}

void Ternary::validate() const {
  if (!cond->is_boolean())
    throw Error("ternary condition is not a boolean", cond->loc);
}

BinaryExpr::BinaryExpr(std::shared_ptr<Expr> lhs_, std::shared_ptr<Expr> rhs_,
  const location &loc_):
  Expr(loc_), lhs(lhs_), rhs(rhs_) {
}

BinaryExpr::BinaryExpr(const BinaryExpr &other):
  Expr(other), lhs(other.lhs->clone()), rhs(other.rhs->clone()) {
}

void swap(BinaryExpr &x, BinaryExpr &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.lhs, y.lhs);
  swap(x.rhs, y.rhs);
}

bool BinaryExpr::constant() const {
  return lhs->constant() && rhs->constant();
}

BooleanBinaryExpr::BooleanBinaryExpr(std::shared_ptr<Expr> lhs_,
  std::shared_ptr<Expr> rhs_, const location &loc_):
  BinaryExpr(lhs_, rhs_, loc_) {
  validate();
}

void BooleanBinaryExpr::validate() const {
  if (!lhs->is_boolean())
    throw Error("left hand side of expression is not a boolean", lhs->loc);

  if (!rhs->is_boolean())
    throw Error("right hand side of expression is not a boolean",
      rhs->loc);
}

Implication &Implication::operator=(Implication other) {
  swap(*this, other);
  return *this;
}

Implication *Implication::clone() const {
  return new Implication(*this);
}

const TypeExpr *Implication::type() const {
  return Boolean.get();
}

void Implication::generate_rvalue(std::ostream &out) const {
  out << "(!" << *lhs << " || " << *rhs << ")";
}

mpz_class Implication::constant_fold() const {
  return !lhs->constant_fold() || rhs->constant_fold();
}

bool Implication::operator==(const Node &other) const {
  auto o = dynamic_cast<const Implication*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

Or &Or::operator=(Or other) {
  swap(*this, other);
  return *this;
}

Or *Or::clone() const {
  return new Or(*this);
}

const TypeExpr *Or::type() const {
  return Boolean.get();
}

void Or::generate_rvalue(std::ostream &out) const {
  out << "(" << *lhs << " || " << *rhs << ")";
}

mpz_class Or::constant_fold() const {
  return lhs->constant_fold() || rhs->constant_fold();
}

bool Or::operator==(const Node &other) const {
  auto o = dynamic_cast<const Or*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

And &And::operator=(And other) {
  swap(*this, other);
  return *this;
}

And *And::clone() const {
  return new And(*this);
}

const TypeExpr *And::type() const {
  return Boolean.get();
}

void And::generate_rvalue(std::ostream &out) const {
  out << "(" << *lhs << " && " << *rhs << ")";
}

mpz_class And::constant_fold() const {
  return lhs->constant_fold() && rhs->constant_fold();
}

bool And::operator==(const Node &other) const {
  auto o = dynamic_cast<const And*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

UnaryExpr::UnaryExpr(std::shared_ptr<Expr> rhs_, const location &loc_):
  Expr(loc_), rhs(rhs_) {
}

UnaryExpr::UnaryExpr(const UnaryExpr &other):
  Expr(other), rhs(other.rhs->clone()) {
}

void swap(UnaryExpr &x, UnaryExpr &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.rhs, y.rhs);
}

bool UnaryExpr::constant() const {
  return rhs->constant();
}

Not::Not(std::shared_ptr<Expr> rhs_, const location &loc_):
  UnaryExpr(rhs_, loc_) {
  validate();
}

Not &Not::operator=(Not other) {
  swap(*this, other);
  return *this;
}

Not *Not::clone() const {
  return new Not(*this);
}

const TypeExpr *Not::type() const {
  return Boolean.get();
}

void Not::generate_rvalue(std::ostream &out) const {
  out << "(!" << *rhs << ")";
}

mpz_class Not::constant_fold() const {
  return !rhs->constant_fold();
}

bool Not::operator==(const Node &other) const {
  auto o = dynamic_cast<const Not*>(&other);
  return o != nullptr && *rhs == *o->rhs;
}

void Not::validate() const {
  if (!rhs->is_boolean())
    throw Error("argument to ! is not a boolean", rhs->loc);
}

static bool comparable(const Expr &lhs, const Expr &rhs) {

  if (lhs.type() == nullptr) {
    // LHS is a numeric literal

    if (rhs.type() == nullptr)
      return true;

    const TypeExpr *t = rhs.type()->resolve();

    if (dynamic_cast<const Range*>(t) != nullptr)
      return true;

    return false;
  }
  
  if (rhs.type() == nullptr) {
    // RHS is a numeric literal

    const TypeExpr *t = lhs.type()->resolve();

    if (dynamic_cast<const Range*>(t) != nullptr)
      return true;

    return false;
  }

  const TypeExpr *t1 = lhs.type()->resolve();
  const TypeExpr *t2 = rhs.type()->resolve();

  if (dynamic_cast<const Range*>(t1) != nullptr) {
    if (dynamic_cast<const Range*>(t2) != nullptr)
      return true;
  }

  return false;
}

ComparisonBinaryExpr::ComparisonBinaryExpr(std::shared_ptr<Expr> lhs_,
  std::shared_ptr<Expr> rhs_, const location &loc_):
  BinaryExpr(lhs_, rhs_, loc_) {
  validate();
}

void ComparisonBinaryExpr::validate() const {
  if (!comparable(*lhs, *rhs))
    throw Error("expressions are not comparable", loc);
}

Lt &Lt::operator=(Lt other) {
  swap(*this, other);
  return *this;
}

Lt *Lt::clone() const {
  return new Lt(*this);
}

const TypeExpr *Lt::type() const {
  return Boolean.get();
}

void Lt::generate_rvalue(std::ostream &out) const {
  out << "(" << *lhs << " < " << *rhs << ")";
}

mpz_class Lt::constant_fold() const {
  return lhs->constant_fold() < rhs->constant_fold();
}

bool Lt::operator==(const Node &other) const {
  auto o = dynamic_cast<const Lt*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

Leq &Leq::operator=(Leq other) {
  swap(*this, other);
  return *this;
}

Leq *Leq::clone() const {
  return new Leq(*this);
}

const TypeExpr *Leq::type() const {
  return Boolean.get();
}

void Leq::generate_rvalue(std::ostream &out) const {
  out << "(" << *lhs << " <= " << *rhs << ")";
}

mpz_class Leq::constant_fold() const {
  return lhs->constant_fold() <= rhs->constant_fold();
}

bool Leq::operator==(const Node &other) const {
  auto o = dynamic_cast<const Leq*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

Gt &Gt::operator=(Gt other) {
  swap(*this, other);
  return *this;
}

Gt *Gt::clone() const {
  return new Gt(*this);
}

const TypeExpr *Gt::type() const {
  return Boolean.get();
}

void Gt::generate_rvalue(std::ostream &out) const {
  out << "(" << *lhs << " > " << *rhs << ")";
}

mpz_class Gt::constant_fold() const {
  return lhs->constant_fold() > rhs->constant_fold();
}

bool Gt::operator==(const Node &other) const {
  auto o = dynamic_cast<const Gt*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

Geq &Geq::operator=(Geq other) {
  swap(*this, other);
  return *this;
}

Geq *Geq::clone() const {
  return new Geq(*this);
}

const TypeExpr *Geq::type() const {
  return Boolean.get();
}

void Geq::generate_rvalue(std::ostream &out) const {
  out << "(" << *lhs << " >= " << *rhs << ")";
}

mpz_class Geq::constant_fold() const {
  return lhs->constant_fold() >= rhs->constant_fold();
}

bool Geq::operator==(const Node &other) const {
  auto o = dynamic_cast<const Geq*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

static bool equatable(const Expr &lhs, const Expr &rhs) {

  if (lhs.type() == nullptr) {
    // LHS is a numeric literal

    if (rhs.type() == nullptr)
      return true;

    const TypeExpr *t = rhs.type()->resolve();

    if (dynamic_cast<const Range*>(t) != nullptr)
      return true;

    if (dynamic_cast<const Scalarset*>(t) != nullptr)
      return true;

    return false;
  }
  
  if (rhs.type() == nullptr) {
    // RHS is a numeric literal

    const TypeExpr *t = lhs.type()->resolve();

    if (dynamic_cast<const Range*>(t) != nullptr)
      return true;

    if (dynamic_cast<const Scalarset*>(t) != nullptr)
      return true;

    return false;
  }

  const TypeExpr *t1 = lhs.type()->resolve();
  const TypeExpr *t2 = rhs.type()->resolve();

  if (dynamic_cast<const Range*>(t1) != nullptr) {
    if (dynamic_cast<const Range*>(t2) != nullptr)
      return true;
  }

  if (auto e1 = dynamic_cast<const Enum*>(t1)) {
    if (auto e2 = dynamic_cast<const Enum*>(t2))
      return *e1 == *e2;
  }

  if (auto s1 = dynamic_cast<const Scalarset*>(t1)) {
    if (auto s2 = dynamic_cast<const Scalarset*>(t2))
      return *s1 == *s2;
  }

  return false;
}

EquatableBinaryExpr::EquatableBinaryExpr(std::shared_ptr<Expr> lhs_,
  std::shared_ptr<Expr> rhs_, const location &loc_):
  BinaryExpr(lhs_, rhs_, loc_) {
  validate();
}

void EquatableBinaryExpr::validate() const {
  if (!equatable(*lhs, *rhs))
    throw Error("expressions are not comparable", loc);
}

Eq &Eq::operator=(Eq other) {
  swap(*this, other);
  return *this;
}

Eq *Eq::clone() const {
  return new Eq(*this);
}

const TypeExpr *Eq::type() const {
  return Boolean.get();
}

void Eq::generate_rvalue(std::ostream &out) const {
  out << "(" << *lhs << " == " << *rhs << ")";
}

mpz_class Eq::constant_fold() const {
  return lhs->constant_fold() == rhs->constant_fold();
}

bool Eq::operator==(const Node &other) const {
  auto o = dynamic_cast<const Eq*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

Neq &Neq::operator=(Neq other) {
  swap(*this, other);
  return *this;
}

Neq *Neq::clone() const {
  return new Neq(*this);
}

const TypeExpr *Neq::type() const {
  return Boolean.get();
}

void Neq::generate_rvalue(std::ostream &out) const {
  out << "(" << *lhs << " != " << *rhs << ")";
}

mpz_class Neq::constant_fold() const {
  return lhs->constant_fold() != rhs->constant_fold();
}

bool Neq::operator==(const Node &other) const {
  auto o = dynamic_cast<const Neq*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

static bool arithmetic(const Expr &lhs, const Expr &rhs) {

  if (lhs.type() == nullptr) {
    // LHS is a numeric literal

    if (rhs.type() == nullptr)
      return true;

    const TypeExpr *t = rhs.type()->resolve();

    if (dynamic_cast<const Range*>(t) != nullptr)
      return true;

    return false;
  }
  
  if (rhs.type() == nullptr) {
    // RHS is a numeric literal

    const TypeExpr *t = lhs.type()->resolve();

    if (dynamic_cast<const Range*>(t) != nullptr)
      return true;

    return false;
  }

  const TypeExpr *t1 = lhs.type()->resolve();
  const TypeExpr *t2 = rhs.type()->resolve();

  if (auto r1 = dynamic_cast<const Range*>(t1)) {
    if (auto r2 = dynamic_cast<const Range*>(t2))
      return *r1 == *r2;
  }

  return false;
}

static std::string lower_bound(const TypeExpr *t) {

  if (t == nullptr)
    return "VALUE_MIN";

  return t->lower_bound();
}

static std::string lower_bound(const TypeExpr *t1, const TypeExpr *t2) {

  if (t1 == nullptr)
    return lower_bound(t2);

  return lower_bound(t1);
}

static std::string lower_bound(const Expr &lhs, const Expr &rhs) {
  return lower_bound(lhs.type(), rhs.type());
}

static std::string lower_bound(const Expr &rhs) {
  return lower_bound(rhs.type());
}

static std::string upper_bound(const TypeExpr *t) {

  if (t == nullptr)
    return "VALUE_MAX";

  return t->upper_bound();
}

static std::string upper_bound(const TypeExpr *t1, const TypeExpr *t2) {

  if (t1 == nullptr)
    return upper_bound(t2);

  return upper_bound(t1);
}

static std::string upper_bound(const Expr &lhs, const Expr &rhs) {
  return upper_bound(lhs.type(), rhs.type());
}

static std::string upper_bound(const Expr &rhs) {
  return upper_bound(rhs.type());
}

ArithmeticBinaryExpr::ArithmeticBinaryExpr(std::shared_ptr<Expr> lhs_,
  std::shared_ptr<Expr> rhs_, const location &loc_):
  BinaryExpr(lhs_, rhs_, loc_) {
  validate();
}

void ArithmeticBinaryExpr::validate() const {
  if (!arithmetic(*lhs, *rhs))
    throw Error("expressions are incompatible in arithmetic expression",
      loc);
}

Add &Add::operator=(Add other) {
  swap(*this, other);
  return *this;
}

Add *Add::clone() const {
  return new Add(*this);
}

const TypeExpr *Add::type() const {
  return nullptr;
}

void Add::generate_rvalue(std::ostream &out) const {
  out << "add(s, " << lower_bound(*lhs, *rhs) << ", " << upper_bound(*lhs, *rhs)
    << ", " << *lhs << ", " << *rhs << ")";
}

mpz_class Add::constant_fold() const {
  return lhs->constant_fold() + rhs->constant_fold();
}

bool Add::operator==(const Node &other) const {
  auto o = dynamic_cast<const Add*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

Sub &Sub::operator=(Sub other) {
  swap(*this, other);
  return *this;
}

Sub *Sub::clone() const {
  return new Sub(*this);
}

const TypeExpr *Sub::type() const {
  return nullptr;
}

void Sub::generate_rvalue(std::ostream &out) const {
  out << "sub(s, " << lower_bound(*lhs, *rhs) << ", " << upper_bound(*lhs, *rhs)
    << ", " << *lhs << ", " << *rhs << ")";
}

mpz_class Sub::constant_fold() const {
  return lhs->constant_fold() - rhs->constant_fold();
}

bool Sub::operator==(const Node &other) const {
  auto o = dynamic_cast<const Sub*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

Negative::Negative(std::shared_ptr<Expr> rhs_, const location &loc_):
  UnaryExpr(rhs_, loc_) {
  validate();
}

void Negative::validate() const {
  if (rhs->type() != nullptr && dynamic_cast<const Range*>(rhs->type()->resolve()) != nullptr)
    throw Error("expression cannot be negated", rhs->loc);
}

Negative &Negative::operator=(Negative other) {
  swap(*this, other);
  return *this;
}

Negative *Negative::clone() const {
  return new Negative(*this);
}

const TypeExpr *Negative::type() const {
  return rhs->type();
}

void Negative::generate_rvalue(std::ostream &out) const {
  out << "negate(s, " << lower_bound(*rhs) << ", " << upper_bound(*rhs) << ", "
    << *rhs << ")";
}

mpz_class Negative::constant_fold() const {
  return -rhs->constant_fold();
}

bool Negative::operator==(const Node &other) const {
  auto o = dynamic_cast<const Negative*>(&other);
  return o != nullptr && *rhs == *o->rhs;
}

Mul &Mul::operator=(Mul other) {
  swap(*this, other);
  return *this;
}

Mul *Mul::clone() const {
  return new Mul(*this);
}

const TypeExpr *Mul::type() const {
  return nullptr;
}

void Mul::generate_rvalue(std::ostream &out) const {
  out << "mul(s, " << lower_bound(*lhs, *rhs) << ", " << upper_bound(*lhs, *rhs)
    << ", " << *lhs << ", " << *rhs << ")";
}

mpz_class Mul::constant_fold() const {
  return lhs->constant_fold() * rhs->constant_fold();
}

bool Mul::operator==(const Node &other) const {
  auto o = dynamic_cast<const Mul*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

Div &Div::operator=(Div other) {
  swap(*this, other);
  return *this;
}

Div *Div::clone() const {
  return new Div(*this);
}

const TypeExpr *Div::type() const {
  return nullptr;
}

void Div::generate_rvalue(std::ostream &out) const {
  out << "divide(s, " << lower_bound(*lhs, *rhs) << ", " <<
    upper_bound(*lhs, *rhs) << ", " << *lhs << ", " << *rhs << ")";
}

mpz_class Div::constant_fold() const {
  mpz_class a = lhs->constant_fold();
  mpz_class b = rhs->constant_fold();
  if (b == 0)
    throw Error("division by 0 in " + a.get_str() + " / " + b.get_str(), loc);
  return a / b;
}

bool Div::operator==(const Node &other) const {
  auto o = dynamic_cast<const Div*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

Mod &Mod::operator=(Mod other) {
  swap(*this, other);
  return *this;
}

Mod *Mod::clone() const {
  return new Mod(*this);
}

const TypeExpr *Mod::type() const {
  return nullptr;
}

void Mod::generate_rvalue(std::ostream &out) const {
  out << "mod(s, " << lower_bound(*lhs, *rhs) << ", " << upper_bound(*lhs, *rhs)
    << ", " << *lhs << ", " << *rhs << ")";
}

mpz_class Mod::constant_fold() const {
  mpz_class a = lhs->constant_fold();
  mpz_class b = rhs->constant_fold();
  if (b == 0)
    throw Error("mod by 0 in " + a.get_str() + " % " + b.get_str(), loc);
  return a % b;
}

bool Mod::operator==(const Node &other) const {
  auto o = dynamic_cast<const Mod*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

void Lvalue::generate_rvalue(std::ostream &out) const {
  generate(out, false);
}

void Lvalue::generate_lvalue(std::ostream &out) const {
  generate(out, true);
}

/* Cheap trick: this destructor is pure virtual in the class declaration, making
 * the class abstract.
 */
Lvalue::~Lvalue() {
}

ExprID::ExprID(const std::string &id_, const std::shared_ptr<Decl> value_,
  const location &loc_):
  Lvalue(loc_), id(id_), value(value_) {
}

ExprID::ExprID(const ExprID &other):
  Lvalue(other), id(other.id), value(other.value->clone()) {
}

ExprID &ExprID::operator=(ExprID other) {
  swap(*this, other);
  return *this;
}

void swap(ExprID &x, ExprID &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.id, y.id);
  swap(x.value, y.value);
}

ExprID *ExprID::clone() const {
  return new ExprID(*this);
}

bool ExprID::constant() const {
  return dynamic_cast<const ConstDecl*>(value.get()) != nullptr;
}

const TypeExpr *ExprID::type() const {
  if (dynamic_cast<const ConstDecl*>(value.get()) != nullptr) {
    return nullptr;
  } else if (auto t = dynamic_cast<const TypeDecl*>(value.get())) {
    return t->value->resolve();
  } else if (auto v = dynamic_cast<const VarDecl*>(value.get())) {
    return v->type->resolve();
  }
  assert(!"unreachable");
  __builtin_unreachable();
}

void ExprID::generate(std::ostream &out, bool lvalue) const {

  // Case 1: this is a reference to a const.
  if (auto c = dynamic_cast<const ConstDecl*>(value.get())) {
    assert(!lvalue && "const appearing as an lvalue");
    out << "ru_" << id;
    return;
  }

  if (auto v = dynamic_cast<const VarDecl*>(value.get())) {

    const TypeExpr *t = type()->resolve();
    assert(t != nullptr && "untyped literal somehow an identifier");

    // Case 2, this is a state variable.
    if (v->state_variable) {

      /* If this is a scalar and we're in an rvalue context, we want to actually
       * read the value of the variable out into an unboxed value as this is
       * what our parent will be expecting.
       */
      if (!lvalue && t->is_simple()) {
        const std::string lb = t->lower_bound();
        const std::string ub = t->upper_bound();
        out << "handle_read(s, " << lb << ", " << ub << ", ";
      }

      /* Note that we need to cast away the const-ness of `s->data` here because
       * we may be within a guard or invariant. In such a situation, the state
       * remains morally read-only.
       */
      out << "((struct handle){ .base = (uint8_t*)s->data, .offset = "
        << v->offset << "ul, .width = " << t->width() << "ul })";

      if (!lvalue && t->is_simple())
        out << ")";
      return;

    }

    // Case 3, this is a local variable
    else {

      if (!lvalue && t->is_simple()) {
        const std::string lb = t->lower_bound();
        const std::string ub = t->upper_bound();
        out << "handle_read(s, " << lb << ", " << ub << ", ";
      }

      out << "ru_" << id;

      if (!lvalue && t->is_simple())
        out << ")";
      return;
    }
  }

  // Case 4, this is an enum member.
  if (dynamic_cast<const TypeDecl*>(value.get())) {
    const TypeExpr *t = type()->resolve();
    assert(t != nullptr && "untyped literal somehow an identifier");

    if (auto e = dynamic_cast<const Enum*>(t)) {
      assert(!lvalue && "enum member appearing as an lvalue");
      size_t i = 0;
      for (const std::pair<std::string, location> &m : e->members) {
        if (id == m.first) {
          out << "VALUE_C(" << i << ")";
          return;
        }
        i++;
      }
      assert(false && "identifier references an enum member that does not exist");
    }
  }

  // FIXME: there's another case here where it's a reference to a quanitified
  // variable. I suspect we should just handle that the same way as a local.
}

mpz_class ExprID::constant_fold() const {
  if (auto c = dynamic_cast<const ConstDecl*>(value.get()))
    return c->value->constant_fold();
  throw Error("symbol \"" + id + "\" is not a constant", loc);
}

bool ExprID::operator==(const Node &other) const {
  auto o = dynamic_cast<const ExprID*>(&other);
  return o != nullptr && id == o->id && *value == *o->value;
}

Field::Field(std::shared_ptr<Lvalue> record_, const std::string &field_,
  const location &loc_):
  Lvalue(loc_), record(record_), field(field_) {
}

Field::Field(const Field &other):
  Lvalue(other), record(other.record->clone()), field(other.field) {
}

void swap(Field &x, Field &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.record, y.record);
  swap(x.field, y.field);
}

Field &Field::operator=(Field other) {
  swap(*this, other);
  return *this;
}

Field *Field::clone() const {
  return new Field(*this);
}

bool Field::constant() const {
  return record->constant();
}

const TypeExpr *Field::type() const {
  const TypeExpr *root = record->type();
  assert(root != nullptr);
  const TypeExpr *resolved = root->resolve();
  assert(resolved != nullptr);
  if (auto r = dynamic_cast<const Record*>(resolved)) {
    for (const std::shared_ptr<VarDecl> &f : r->fields) {
      if (f->name == field)
        return f->type.get();
    }
    throw Error("no field named \"" + field + "\" in record", loc);
  }
  throw Error("left hand side of field expression is not a record", loc);
}

void Field::generate(std::ostream &out, bool lvalue) const {
  const TypeExpr *root = record->type();
  assert(root != nullptr);
  const TypeExpr *resolved = root->resolve();
  assert(resolved != nullptr);
  if (auto r = dynamic_cast<const Record*>(resolved)) {
    mpz_class offset = 0;
    for (const std::shared_ptr<VarDecl> &f : r->fields) {
      if (f->name == field) {
        if (!lvalue && f->type->is_simple()) {
          const std::string lb = f->type->lower_bound();
          const std::string ub = f->type->upper_bound();
          out << "handle_read(s, " << lb << ", " << ub << ", ";
        }
        out << "handle_narrow(";
        record->generate(out, lvalue);
        out << ", " << offset << ", " << f->type->width() << ")";
        if (!lvalue && f->type->is_simple())
          out << ")";
        return;
      }
      offset += f->type->width();
    }
    throw Error("no field named \"" + field + "\" in record", loc);
  }
  throw Error("left hand side of field expression is not a record", loc);
}

mpz_class Field::constant_fold() const {
  throw Error("field expression used in constant", loc);
}

bool Field::operator==(const Node &other) const {
  auto o = dynamic_cast<const Field*>(&other);
  return o != nullptr && *record == *o->record && field == o->field;
}

Element::Element(std::shared_ptr<Lvalue> array_, std::shared_ptr<Expr> index_,
  const location &loc_):
  Lvalue(loc_), array(array_), index(index_) {
}

Element::Element(const Element &other):
  Lvalue(other), array(other.array->clone()), index(other.index->clone()) {
}

void swap(Element &x, Element &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.array, y.array);
  swap(x.index, y.index);
}

Element &Element::operator=(Element other) {
  swap(*this, other);
  return *this;
}

Element *Element::clone() const {
  return new Element(*this);
}

bool Element::constant() const {
  return array->constant() && index->constant();
}

const TypeExpr *Element::type() const {
  const TypeExpr *t = array->type()->resolve();
  const Array *a = dynamic_cast<const Array*>(t);
  assert(a != nullptr &&
    "array reference based on something that is not an array");
  return a->element_type.get();
}

void Element::generate(std::ostream &out, bool lvalue) const {

  // First, determine the width of the array's elements

  const TypeExpr *t1 = array->type();
  assert(t1 != nullptr && "array with invalid type");
  const TypeExpr *t2 = t1->resolve();
  assert(t2 != nullptr && "array with invalid type");

  auto a = dynamic_cast<const Array&>(*t2);
  mpz_class element_width = a.element_type->width();

  // Second, determine the minimum and maximum values of the array's index type

  const TypeExpr *t3 = a.index_type->resolve();
  assert(t3 != nullptr && "array with invalid index type");

  mpz_class min, max;
  if (auto r = dynamic_cast<const Range*>(t3)) {
    min = r->min->constant_fold();
    max = r->max->constant_fold();
  } else if (auto e = dynamic_cast<const Enum*>(t3)) {
    min = 0;
    max = e->count() - 1;
  } else if (auto s = dynamic_cast<const Scalarset*>(t3)) {
    min = 0;
    max = s->bound->constant_fold() - 1;
  } else {
    assert(false && "array with invalid index type");
  }

  if (!lvalue && a.element_type->is_simple()) {
    const std::string lb = a.element_type->lower_bound();
    const std::string ub = a.element_type->upper_bound();
    out << "handle_read(s, " << lb << ", " << ub << ", ";
  }

  out << "handle_index(s, SIZE_C(" << element_width << "), VALUE_C(" << min
    << "), VALUE_C(" << max << "), ";
  array->generate(out, lvalue);
  out << ", ";
  index->generate_rvalue(out);
  out << ")";

  if (!lvalue && a.element_type->is_simple())
    out << ")";
}

mpz_class Element::constant_fold() const {
  throw Error("array element used in constant", loc);
}

bool Element::operator==(const Node &other) const {
  auto o = dynamic_cast<const Element*>(&other);
  return o != nullptr && *array == *o->array && *index == *o->index;
}

FunctionCall::FunctionCall(std::shared_ptr<Function> function_,
  std::vector<std::shared_ptr<Expr>> arguments_, const location &loc_):
  Expr(loc_), function(function_), arguments(arguments_) { }

FunctionCall::FunctionCall(const FunctionCall &other):
  Expr(other), function(other.function->clone()) {

  for (const std::shared_ptr<Expr> &a : other.arguments)
    arguments.emplace_back(a->clone());
}

void swap(FunctionCall &x, FunctionCall &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.function, y.function);
  swap(x.arguments, y.arguments);
}

FunctionCall &FunctionCall::operator=(FunctionCall other) {
  swap(*this, other);
  return *this;
}

FunctionCall *FunctionCall::clone() const {
  return new FunctionCall(*this);
}

bool FunctionCall::constant() const {
  /* TODO: For now, we conservatively treat function calls as non-constant. In
   * future, it would be nice to lift this restriction to support more advanced
   * parameterised models.
   */
  return false;
}

const TypeExpr *FunctionCall::type() const {
  return function->return_type.get();
}

void FunctionCall::generate_rvalue(std::ostream&) const {
  // TODO: We need to decide how we're going to handle return values
  assert("!TODO");
}

mpz_class FunctionCall::constant_fold() const {
  // See FunctionCall::constant() regarding conservatism here.
  throw Error("function call used in a constant", loc);
}

bool FunctionCall::operator==(const Node &other) const {
  auto o = dynamic_cast<const FunctionCall*>(&other);
  if (o == nullptr)
    return false;
  if (*function != *o->function)
    return false;
  if (!vector_eq(arguments, o->arguments))
    return false;
  return true;
}

Quantifier::Quantifier(const std::string &name, std::shared_ptr<TypeExpr> type,
  const location &loc_)
  : Node(loc_), var(std::make_shared<VarDecl>(name, type, loc_)), step(nullptr) {
}

Quantifier::Quantifier(const std::string &name, std::shared_ptr<Expr> from,
  std::shared_ptr<Expr> to, const location &loc_)
  : Quantifier(loc_, name, from, to, nullptr) {
}

Quantifier::Quantifier(const std::string &name, std::shared_ptr<Expr> from,
  std::shared_ptr<Expr> to, std::shared_ptr<Expr> step_,
  const location &loc_)
  : Quantifier(loc_, name, from, to, step_) {
}

Quantifier::Quantifier(const location &loc_, const std::string &name,
  std::shared_ptr<Expr> from, std::shared_ptr<Expr> to,
  std::shared_ptr<Expr> step_):
  Node(loc_),
  var(std::make_shared<VarDecl>(name, std::make_shared<Range>(from, to, loc_), loc_)),
  step(step_) {
}

Quantifier::Quantifier(const Quantifier &other):
  Node(other), var(other.var->clone()),
  step(other.step == nullptr ? nullptr : other.step->clone()) {
}

Quantifier &Quantifier::operator=(Quantifier other) {
  swap(*this, other);
  return *this;
}

void swap(Quantifier &x, Quantifier &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.var, y.var);
  swap(x.step, y.step);
}

Quantifier *Quantifier::clone() const {
  return new Quantifier(*this);
}

void Quantifier::generate_header(std::ostream &out) const {

  std::string const counter = "_ru1_" + var->name;
  std::string const lb = var->type->lower_bound();
  std::string const ub = var->type->upper_bound();
  std::string const inc = step == nullptr
    ? "VALUE_C(1)"
    : "VALUE_C(" + step->constant_fold().get_str() + ")";

  std::string const block = "_ru2_" + var->name;
  mpz_class width = var->type->width();

  std::string const handle = "ru_" + var->name;

  /* Set up quantifiers. It might be surprising to notice that there is an extra
   * level of indirection here. A variable 'x' results in loop counter '_ru1_x',
   * storage array '_ru2_x' and handle 'ru_x'. We use three variables rather
   * than two in order to avoid rules that modify the ruleset parameters
   * (uncommon) affecting the loop counter.
   */
  out
    << "for (value_t " << counter << " = " << lb << "; " << counter << " <= "
      << ub << "; " << counter << " += " << inc << ") {\n"
    << "  uint8_t " << block << "[BITS_TO_BYTES(" << width << ")] = { 0 };\n"
    << "  struct handle " << handle << " = { .base = " << block
      << ", .offset = 0, .width = SIZE_C(" << width << ") };\n"
    << "  handle_write(s, " << lb << ", " << ub << ", " << handle << ", "
      << counter << ");\n";
}

void Quantifier::generate_footer(std::ostream &out) const {

  std::string const counter = "_ru1_" + var->name;
  std::string const ub = var->type->upper_bound();
  std::string const inc = step == nullptr
    ? "VALUE_C(1)"
    : "VALUE_C(" + step->constant_fold().get_str() + ")";

  out
    << "  if (VALUE_MAX - " << inc << " < " << ub << " && " << counter
      << " > VALUE_MAX - " << inc << ") {\n"
    << "    break;\n"
    << "  }\n"
    << "}\n";
}

bool Quantifier::operator==(const Node &other) const {
  auto o = dynamic_cast<const Quantifier*>(&other);
  if (o == nullptr)
    return false;
  if (*var != *o->var)
    return false;
  if (step == nullptr) {
    if (o->step != nullptr)
      return false;
  } else {
    if (o->step == nullptr || *step != *o->step)
      return false;
  }
  return true;
}

Exists::Exists(std::shared_ptr<Quantifier> quantifier_,
  std::shared_ptr<Expr> expr_, const location &loc_):
  Expr(loc_), quantifier(quantifier_), expr(expr_) {
  validate();
}

Exists::Exists(const Exists &other):
  Expr(other), quantifier(other.quantifier->clone()), expr(other.expr->clone()) {
}

Exists &Exists::operator=(Exists other) {
  swap(*this, other);
  return *this;
}

void swap(Exists &x, Exists &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.quantifier, y.quantifier);
  swap(x.expr, y.expr);
}

Exists *Exists::clone() const {
  return new Exists(*this);
}

bool Exists::constant() const {
  return expr->constant();
}

const TypeExpr *Exists::type() const {
  return Boolean.get();
}

bool Exists::operator==(const Node &other) const {
  auto o = dynamic_cast<const Exists*>(&other);
  return o != nullptr && *quantifier == *o->quantifier && *expr == *o->expr;
}

void Exists::generate_rvalue(std::ostream &out) const {
  out << "({ bool result = false; ";
  quantifier->generate_header(out);
  out << "if (";
  expr->generate_rvalue(out);
  out << ") { result = true; break; }";
  quantifier->generate_footer(out);
  out << " result; })";
}

mpz_class Exists::constant_fold() const {
  throw Error("exists expression used in constant", loc);
}

void Exists::validate() const {
  if (!expr->is_boolean())
    throw Error("expression in exists is not boolean", expr->loc);
}

Forall::Forall(std::shared_ptr<Quantifier> quantifier_,
  std::shared_ptr<Expr> expr_, const location &loc_):
  Expr(loc_), quantifier(quantifier_), expr(expr_) {
  validate();
}

Forall::Forall(const Forall &other):
  Expr(other), quantifier(other.quantifier->clone()), expr(other.expr->clone()) {
}

Forall &Forall::operator=(Forall other) {
  swap(*this, other);
  return *this;
}

void swap(Forall &x, Forall &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.quantifier, y.quantifier);
  swap(x.expr, y.expr);
}

Forall *Forall::clone() const {
  return new Forall(*this);
}

bool Forall::constant() const {
  return expr->constant();
}

const TypeExpr *Forall::type() const {
  return Boolean.get();
}

void Forall::generate_rvalue(std::ostream &out) const {
  out << "({ bool result = true; ";
  quantifier->generate_header(out);
  out << "if (!";
  expr->generate_rvalue(out);
  out << ") { result = false; break; }";
  quantifier->generate_footer(out);
  out << " result; })";
}

mpz_class Forall::constant_fold() const {
  throw Error("forall expression used in constant", loc);
}

bool Forall::operator==(const Node &other) const {
  auto o = dynamic_cast<const Forall*>(&other);
  return o != nullptr && *quantifier == *o->quantifier && *expr == *o->expr;
}

void Forall::validate() const {
  if (!expr->is_boolean())
    throw Error("expression in forall is not boolean", expr->loc);
}

}
