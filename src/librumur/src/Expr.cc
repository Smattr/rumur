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

void Ternary::generate(std::ostream &out) const {
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

void Implication::generate(std::ostream &out) const {
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

void Or::generate(std::ostream &out) const {
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

void And::generate(std::ostream &out) const {
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

void Not::generate(std::ostream &out) const {
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

void Lt::generate(std::ostream &out) const {
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

void Leq::generate(std::ostream &out) const {
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

void Gt::generate(std::ostream &out) const {
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

void Geq::generate(std::ostream &out) const {
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

void Eq::generate(std::ostream &out) const {
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

void Neq::generate(std::ostream &out) const {
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

void Add::generate(std::ostream &out) const {
  out << "(" << *lhs << " + " << *rhs << ")";
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

void Sub::generate(std::ostream &out) const {
  out << "(" << *lhs << " - " << *rhs << ")";
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

void Negative::generate(std::ostream &out) const {
  out << "(-" << *rhs << ")";
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

void Mul::generate(std::ostream &out) const {
  out << "(" << *lhs << " * " << *rhs << ")";
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

void Div::generate(std::ostream &out) const {
  out << "(" << *lhs << " / " << *rhs << ")";
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

void Mod::generate(std::ostream &out) const {
  out << "(" << *lhs << " % " << *rhs << ")";
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

void ExprID::generate(std::ostream &out) const {
  if (auto v = dynamic_cast<const VarDecl*>(value)) {
    if (v->state_variable) {
      out << "ru_u_" << id << "::make(s, size_t(" << v->offset << "ul))";
      return;
    }
  }
  const TypeExpr *t = type();
  if (t != nullptr) {
    if (auto e = dynamic_cast<const Enum*>(t)) {
      size_t i = 0;
      for (const std::pair<std::string, location> &m : e->members) {
        if (id == m.first) {
          out << *e << "::value_type(UINT64_C(" << i << "))";
          return;
        }
        i++;
      }
    }
  }
  out << "ru_u_" << id;
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
  // TODO
  return nullptr;
}

void Field::generate(std::ostream &out) const {
  out << "(" << *record << "." << field << ")";
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

void Element::generate(std::ostream &out) const {
  out << "(" << *array << "[" << *index << "])";
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

void Quantifier::generate(std::ostream &out) const {
  // TODO: needs some more work
  out << "for(" << var->name << "...";
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

void Exists::generate(std::ostream &out) const {
  out << "({bool ru_g_TODO=false;" << *quantifier << "{if(" << *expr
    << "){ru_g_TODO=true;break;}}ru_g_TODO;})";
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

void Forall::generate(std::ostream &out) const {
  out << "({bool ru_g_TODO=true;" << *quantifier << "{if(" << *expr
    << "){ru_g_TODO=false;break;}}ru_g_TODO;})";
}

int64_t Forall::constant_fold() const {
  throw Error("forall expression used in constant", loc);
}

bool Forall::operator==(const Node &other) const {
  auto o = dynamic_cast<const Forall*>(&other);
  return o != nullptr && *quantifier == *o->quantifier && *expr == *o->expr;
}

}
