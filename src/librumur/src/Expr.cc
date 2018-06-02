#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include "location.hh"
#include <rumur/Boolean.h>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>

namespace rumur {

bool Expr::is_boolean() const {
  return type() != nullptr && *type()->resolve() == Boolean;
}

Ternary::Ternary(Expr *cond_, Expr *lhs_, Expr *rhs_, const location &loc_):
  Expr(loc_), cond(cond_), lhs(lhs_), rhs(rhs_) {

  if (!cond->is_boolean())
    throw Error("ternary condition is not a boolean", cond->loc);
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

Ternary::~Ternary() {
  delete cond;
  delete lhs;
  delete rhs;
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

int64_t Ternary::constant_fold() const {
  return cond->constant_fold() ? lhs->constant_fold() : rhs->constant_fold();
}

BinaryExpr::BinaryExpr(Expr *lhs_, Expr *rhs_, const location &loc_):
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

BinaryExpr::~BinaryExpr() {
  delete lhs;
  delete rhs;
}

BooleanBinaryExpr::BooleanBinaryExpr(Expr *lhs_, Expr *rhs_, const location &loc_):
  BinaryExpr(lhs_, rhs_, loc_) {
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
  return &Boolean;
}

void Implication::generate_rvalue(std::ostream &out) const {
  out << "(!" << *lhs << " || " << *rhs << ")";
}

int64_t Implication::constant_fold() const {
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
  return &Boolean;
}

void Or::generate_rvalue(std::ostream &out) const {
  out << "(" << *lhs << " || " << *rhs << ")";
}

int64_t Or::constant_fold() const {
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
  return &Boolean;
}

void And::generate_rvalue(std::ostream &out) const {
  out << "(" << *lhs << " && " << *rhs << ")";
}

int64_t And::constant_fold() const {
  return lhs->constant_fold() && rhs->constant_fold();
}

bool And::operator==(const Node &other) const {
  auto o = dynamic_cast<const And*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

UnaryExpr::UnaryExpr(Expr *rhs_, const location &loc_):
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

UnaryExpr::~UnaryExpr() {
  delete rhs;
}

bool UnaryExpr::constant() const {
  return rhs->constant();
}

Not::Not(Expr *rhs_, const location &loc_):
  UnaryExpr(rhs_, loc_) {
  if (!rhs->is_boolean())
    throw Error("argument to ! is not a boolean", rhs->loc);
}

Not &Not::operator=(Not other) {
  swap(*this, other);
  return *this;
}

Not *Not::clone() const {
  return new Not(*this);
}

const TypeExpr *Not::type() const {
  return &Boolean;
}

void Not::generate_rvalue(std::ostream &out) const {
  out << "(!" << *rhs << ")";
}

int64_t Not::constant_fold() const {
  return !rhs->constant_fold();
}

bool Not::operator==(const Node &other) const {
  auto o = dynamic_cast<const Not*>(&other);
  return o != nullptr && *rhs == *o->rhs;
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

  if (auto r1 = dynamic_cast<const Range*>(t1)) {
    if (auto r2 = dynamic_cast<const Range*>(t2))
      return *r1 == *r2;
  }

  return false;
}

ComparisonBinaryExpr::ComparisonBinaryExpr(Expr *lhs_, Expr *rhs_, const location &loc_):
  BinaryExpr(lhs_, rhs_, loc_) {
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
  return &Boolean;
}

void Lt::generate_rvalue(std::ostream &out) const {
  out << "(" << *lhs << " < " << *rhs << ")";
}

int64_t Lt::constant_fold() const {
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
  return &Boolean;
}

void Leq::generate_rvalue(std::ostream &out) const {
  out << "(" << *lhs << " <= " << *rhs << ")";
}

int64_t Leq::constant_fold() const {
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
  return &Boolean;
}

void Gt::generate_rvalue(std::ostream &out) const {
  out << "(" << *lhs << " > " << *rhs << ")";
}

int64_t Gt::constant_fold() const {
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
  return &Boolean;
}

void Geq::generate_rvalue(std::ostream &out) const {
  out << "(" << *lhs << " >= " << *rhs << ")";
}

int64_t Geq::constant_fold() const {
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

  if (auto e1 = dynamic_cast<const Enum*>(t1)) {
    if (auto e2 = dynamic_cast<const Enum*>(t2))
      return *e1 == *e2;
  }

  return false;
}

EquatableBinaryExpr::EquatableBinaryExpr(Expr *lhs_, Expr *rhs_, const location &loc_):
  BinaryExpr(lhs_, rhs_, loc_) {
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
  return &Boolean;
}

void Eq::generate_rvalue(std::ostream &out) const {
  out << "(" << *lhs << " == " << *rhs << ")";
}

int64_t Eq::constant_fold() const {
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
  return &Boolean;
}

void Neq::generate_rvalue(std::ostream &out) const {
  out << "(" << *lhs << " != " << *rhs << ")";
}

int64_t Neq::constant_fold() const {
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

ArithmeticBinaryExpr::ArithmeticBinaryExpr(Expr *lhs_, Expr *rhs_, const location &loc_):
  BinaryExpr(lhs_, rhs_, loc_) {
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

int64_t Add::constant_fold() const {
  int64_t r;
  int64_t a = lhs->constant_fold();
  int64_t b = rhs->constant_fold();
  if (__builtin_add_overflow(a, b, &r))
    throw Error("overflow in " + std::to_string(a) + " + "
      + std::to_string(b), loc);
  return r;
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

int64_t Sub::constant_fold() const {
  int64_t r;
  int64_t a = lhs->constant_fold();
  int64_t b = rhs->constant_fold();
  if (__builtin_sub_overflow(a, b, &r))
    throw Error("overflow in " + std::to_string(a) + " - "
      + std::to_string(b), loc);
  return r;
}

bool Sub::operator==(const Node &other) const {
  auto o = dynamic_cast<const Sub*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

Negative::Negative(Expr *rhs_, const location &loc_):
  UnaryExpr(rhs_, loc_) {
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

int64_t Negative::constant_fold() const {
  int64_t a = rhs->constant_fold();
  if (a == std::numeric_limits<int64_t>::min())
    throw Error("overflow in -" + std::to_string(a), loc);
  return -a;
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

int64_t Mul::constant_fold() const {
  int64_t r;
  int64_t a = lhs->constant_fold();
  int64_t b = rhs->constant_fold();
  if (__builtin_mul_overflow(a, b, &r))
    throw Error("overflow in " + std::to_string(a) + " * "
      + std::to_string(b), loc);
  return r;
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

int64_t Div::constant_fold() const {
  int64_t a = lhs->constant_fold();
  int64_t b = rhs->constant_fold();
  if (b == 0)
    throw Error("division by 0 in " + std::to_string(a) + " / "
      + std::to_string(b), loc);
  if (a == std::numeric_limits<int64_t>::min() && b == -1)
    throw Error("overflow in " + std::to_string(a) + " / "
      + std::to_string(b), loc);
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

int64_t Mod::constant_fold() const {
  int64_t a = lhs->constant_fold();
  int64_t b = rhs->constant_fold();
  if (b == 0)
    throw Error("mod by 0 in " + std::to_string(a) + " % "
      + std::to_string(b), loc);
  if (a == std::numeric_limits<int64_t>::min() && b == -1)
    throw Error("overflow in " + std::to_string(a) + " % "
      + std::to_string(b), loc);
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

ExprID::ExprID(const std::string &id_, const Decl *value_, const location &loc_):
  Lvalue(loc_), id(id_), value(value_->clone()) {
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
  return dynamic_cast<const ConstDecl*>(value) != nullptr;
}

const TypeExpr *ExprID::type() const {
  if (dynamic_cast<const ConstDecl*>(value) != nullptr) {
    return nullptr;
  } else if (auto t = dynamic_cast<const TypeDecl*>(value)) {
    return t->value->resolve();
  } else if (auto v = dynamic_cast<const VarDecl*>(value)) {
    return v->type->resolve();
  }
  assert(!"unreachable");
  __builtin_unreachable();
}

void ExprID::generate(std::ostream &out, bool lvalue) const {

  // FIXME: what if this is a const? Does type() return nullptr?
  const TypeExpr *t = type()->resolve();
  assert(t != nullptr && "untyped literal somehow an identifier");

  if (auto v = dynamic_cast<const VarDecl*>(value)) {

    // Case 1, this is a state variable.
    if (v->state_variable) {

      /* If this is a scalar and we're in an rvalue context, we want to actually
       * read the value of the variable out into an unboxed value as this is
       * what our parent will be expecting.
       */
      if (!lvalue && t->is_simple())
        out << "handle_read(s, ";

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

    // Case 2, this is a local variable
    else {

      if (!lvalue && t->is_simple())
        out << "handle_read(s, ";

      out << "ru_" << id;

      if (!lvalue && t->is_simple())
        out << ")";
      return;
    }
  }

  // Case 3, this is an enum member.
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

  // Case 4, this is a reference to a const
  if (dynamic_cast<const ConstDecl*>(t) != nullptr) {
    assert(!lvalue && "const appearing as an lvalue");
    out << "ru_" << id;
    return;
  }

  // FIXME: there's another case here where it's a reference to a quanitified
  // variable. I suspect we should just handle that the same way as a local.
}

ExprID::~ExprID() {
  delete value;
}

int64_t ExprID::constant_fold() const {
  if (auto c = dynamic_cast<const ConstDecl*>(value))
    return c->value->constant_fold();
  throw Error("symbol \"" + id + "\" is not a constant", loc);
}

bool ExprID::operator==(const Node &other) const {
  auto o = dynamic_cast<const ExprID*>(&other);
  return o != nullptr && id == o->id && *value == *o->value;
}

Field::Field(Lvalue *record_, const std::string &field_, const location &loc_):
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
    for (const VarDecl *f : r->fields) {
      if (f->name == field)
        return f->type;
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
    size_t offset = 0;
    for (const VarDecl *f : r->fields) {
      if (f->name == field) {
        if (!lvalue && f->type->is_simple())
          out << "handle_read(s, ";
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

Field::~Field() {
  delete record;
}

int64_t Field::constant_fold() const {
  throw Error("field expression used in constant", loc);
}

bool Field::operator==(const Node &other) const {
  auto o = dynamic_cast<const Field*>(&other);
  return o != nullptr && *record == *o->record && field == o->field;
}

Element::Element(Lvalue *array_, Expr *index_, const location &loc_):
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

Element::~Element() {
  delete array;
  delete index;
}

bool Element::constant() const {
  return array->constant() && index->constant();
}

const TypeExpr *Element::type() const {
  const TypeExpr *t = array->type()->resolve();
  const Array *a = dynamic_cast<const Array*>(t);
  assert(a != nullptr &&
    "array reference based on something that is not an array");
  return a->element_type;
}

void Element::generate(std::ostream &out, bool lvalue) const {

  // First, determine the width of the array's elements

  const TypeExpr *t1 = array->type();
  assert(t1 != nullptr && "array with invalid type");
  const TypeExpr *t2 = t1->resolve();
  assert(t2 != nullptr && "array with invalid type");

  auto a = dynamic_cast<const Array&>(*t2);
  size_t element_width = a.element_type->width();

  // Second, determine the minimum and maximum values of the array's index type

  const TypeExpr *t3 = a.index_type->resolve();
  assert(t3 != nullptr && "array with invalid index type");

  int64_t min, max;
  if (auto r = dynamic_cast<const Range*>(t3)) {
    min = r->min->constant_fold();
    max = r->max->constant_fold();
  } else if (auto e = dynamic_cast<const Enum*>(t3)) {
    min = 0;
    max = int64_t(e->count()) - 1;
  } else {
    assert(false && "array with invalid index type");
  }

  if (!lvalue && a.element_type->is_simple())
    out << "handle_read(s, ";

  out << "handle_index(s, SIZE_C(" << element_width << "), VALUE_C(" << min
    << "), VALUE_C(" << max << "), ";
  array->generate(out, lvalue);
  out << ", ";
  index->generate_rvalue(out);
  out << ")";

  if (!lvalue && a.element_type->is_simple())
    out << ")";
}

int64_t Element::constant_fold() const {
  throw Error("array element used in constant", loc);
}

bool Element::operator==(const Node &other) const {
  auto o = dynamic_cast<const Element*>(&other);
  return o != nullptr && *array == *o->array && *index == *o->index;
}

Quantifier::Quantifier(const std::string &name, TypeExpr *type,
  const location &loc_)
  : Node(loc_), var(new VarDecl(name, type, loc_)) {
}

Quantifier::Quantifier(const std::string &name, Expr *from, Expr *to,
  const location &loc_)
  : Quantifier(loc_, name, from, to, nullptr) {
}

Quantifier::Quantifier(const std::string &name, Expr *from, Expr *to, Expr *step_,
  const location &loc_)
  : Quantifier(loc_, name, from, to, step_) {
}

Quantifier::Quantifier(const location &loc_, const std::string &name, Expr *from,
  Expr *to, Expr *step_):
  Node(loc_),
  var(new VarDecl(name, new Range(from, to, loc_), loc_)),
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

Quantifier::~Quantifier() {
  delete var;
  delete step;
}

void Quantifier::generate_header(std::ostream &out) const {

  // FIXME: Accesses to this variable within the loop will expect to access it
  // via handle_read, as opposed to it being an immediate int64_t.
  // FIXME: support steps that are not 1
  out << "for (value_t ru_" << var->name << " = " << lower_bound(var->type)
    << "; ; ru_" << var->name << "++) {";
}

void Quantifier::generate_footer(std::ostream &out) const {
  out << "if (ru_" << var->name << " == " << upper_bound(var->type)
    << ") { break; } }";
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

Exists::Exists(Quantifier *quantifier_, Expr *expr_, const location &loc_):
  Expr(loc_), quantifier(quantifier_), expr(expr_) {
  if (!expr->is_boolean())
    throw Error("expression in exists is not boolean", expr->loc);
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
  return &Boolean;
}

Exists::~Exists() {
  delete quantifier;
  delete expr;
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

int64_t Exists::constant_fold() const {
  throw Error("exists expression used in constant", loc);
}

Forall::Forall(Quantifier *quantifier_, Expr *expr_, const location &loc_):
  Expr(loc_), quantifier(quantifier_), expr(expr_) {
  if (!expr->is_boolean())
    throw Error("expression in forall is not boolean", expr->loc);
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
  return &Boolean;
}

Forall::~Forall() {
  delete quantifier;
  delete expr;
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

int64_t Forall::constant_fold() const {
  throw Error("forall expression used in constant", loc);
}

bool Forall::operator==(const Node &other) const {
  auto o = dynamic_cast<const Forall*>(&other);
  return o != nullptr && *quantifier == *o->quantifier && *expr == *o->expr;
}

}
