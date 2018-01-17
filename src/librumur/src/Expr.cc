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
#include <stdexcept>
#include <string>
#include <utility>

namespace rumur {

bool Expr::is_arithmetic() const {

  // Is this a literal?
  if (type() == nullptr)
    return true;

  // Is this of a range type?
  if (dynamic_cast<const Range*>(type()) != nullptr)
    return true;

  return false;
}

static void expect_arithmetic(const Expr *e) {
  if (!e->is_arithmetic())
    throw RumurError("expected arithmetic expression is not arithmetic",
      e->loc);
}

bool Expr::is_boolean() const {
  return type() == &Boolean;
}

static void expect_boolean(const Expr *e) {
  if (!e->is_boolean())
    throw RumurError("expected boolean expression is not a boolean",
      e->loc);
}

Ternary::Ternary(Expr *cond_, Expr *lhs_, Expr *rhs_, const location &loc_):
  Expr(loc_), cond(cond_), lhs(lhs_), rhs(rhs_) {
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

void Ternary::validate() const {
  cond->validate();
  lhs->validate();
  rhs->validate();

  expect_boolean(cond);

  // TODO: check lhs and rhs have the same type
}

bool Ternary::constant() const {
  return cond->constant() && lhs->constant() && rhs->constant();
}

const TypeExpr *Ternary::type() const {
  // TODO: assert lhs and rhs are compatible types.
  return lhs->type();
}

void Ternary::generate(std::ostream &out) const {
  out << "(" << *cond << "?" << *lhs << ":" << *rhs << ")";
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

void BinaryExpr::validate() const {
  lhs->validate();
  rhs->validate();
}

bool BinaryExpr::constant() const {
  return lhs->constant() && rhs->constant();
}

BinaryExpr::~BinaryExpr() {
  delete lhs;
  delete rhs;
}

Implication &Implication::operator=(Implication other) {
  swap(*this, other);
  return *this;
}

Implication *Implication::clone() const {
  return new Implication(*this);
}

void Implication::validate() const {
  BinaryExpr::validate();

  expect_boolean(lhs);
  expect_boolean(rhs);
}

const TypeExpr *Implication::type() const {
  return &Boolean;
}

void Implication::generate(std::ostream &out) const {
  out << "(!" << *lhs << "||" << *rhs << ")";
}

int64_t Implication::constant_fold() const {
  return !lhs->constant_fold() || rhs->constant_fold();
}

Or &Or::operator=(Or other) {
  swap(*this, other);
  return *this;
}

Or *Or::clone() const {
  return new Or(*this);
}

void Or::validate() const {
  BinaryExpr::validate();

  expect_boolean(lhs);
  expect_boolean(rhs);
}

const TypeExpr *Or::type() const {
  return &Boolean;
}

void Or::generate(std::ostream &out) const {
  out << "(" << *lhs << "||" << *rhs << ")";
}

int64_t Or::constant_fold() const {
  return lhs->constant_fold() || rhs->constant_fold();
}

And &And::operator=(And other) {
  swap(*this, other);
  return *this;
}

And *And::clone() const {
  return new And(*this);
}

void And::validate() const {
  BinaryExpr::validate();

  expect_boolean(lhs);
  expect_boolean(rhs);
}

const TypeExpr *And::type() const {
  return &Boolean;
}

void And::generate(std::ostream &out) const {
  out << "(" << *lhs << "&&" << *rhs << ")";
}

int64_t And::constant_fold() const {
  return lhs->constant_fold() && rhs->constant_fold();
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

void UnaryExpr::validate() const {
  rhs->validate();
}

bool UnaryExpr::constant() const {
  return rhs->constant();
}

Not &Not::operator=(Not other) {
  swap(*this, other);
  return *this;
}

Not *Not::clone() const {
  return new Not(*this);
}

void Not::validate() const {
  UnaryExpr::validate();

  expect_boolean(rhs);
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

Lt &Lt::operator=(Lt other) {
  swap(*this, other);
  return *this;
}

Lt *Lt::clone() const {
  return new Lt(*this);
}

void Lt::validate() const {
  BinaryExpr::validate();

  expect_arithmetic(lhs);
  expect_arithmetic(rhs);
}

const TypeExpr *Lt::type() const {
  return &Boolean;
}

void Lt::generate(std::ostream &out) const {
  out << "(" << *lhs << "<" << *rhs << ")";
}

int64_t Lt::constant_fold() const {
  return lhs->constant_fold() < rhs->constant_fold();
}

Leq &Leq::operator=(Leq other) {
  swap(*this, other);
  return *this;
}

Leq *Leq::clone() const {
  return new Leq(*this);
}

void Leq::validate() const {
  BinaryExpr::validate();

  expect_arithmetic(lhs);
  expect_arithmetic(rhs);
}

const TypeExpr *Leq::type() const {
  return &Boolean;
}

void Leq::generate(std::ostream &out) const {
  out << "(" << *lhs << "<=" << *rhs << ")";
}

int64_t Leq::constant_fold() const {
  return lhs->constant_fold() <= rhs->constant_fold();
}

Gt &Gt::operator=(Gt other) {
  swap(*this, other);
  return *this;
}

Gt *Gt::clone() const {
  return new Gt(*this);
}

void Gt::validate() const {
  BinaryExpr::validate();

  expect_arithmetic(lhs);
  expect_arithmetic(rhs);
}

const TypeExpr *Gt::type() const {
  return &Boolean;
}

void Gt::generate(std::ostream &out) const {
  out << "(" << *lhs << ">" << *rhs << ")";
}

int64_t Gt::constant_fold() const {
  return lhs->constant_fold() > rhs->constant_fold();
}

Geq &Geq::operator=(Geq other) {
  swap(*this, other);
  return *this;
}

Geq *Geq::clone() const {
  return new Geq(*this);
}

void Geq::validate() const {
  BinaryExpr::validate();

  expect_arithmetic(lhs);
  expect_arithmetic(rhs);
}

const TypeExpr *Geq::type() const {
  return &Boolean;
}

void Geq::generate(std::ostream &out) const {
  out << "(" << *lhs << ">=" << *rhs << ")";
}

int64_t Geq::constant_fold() const {
  return lhs->constant_fold() >= rhs->constant_fold();
}

Eq &Eq::operator=(Eq other) {
  swap(*this, other);
  return *this;
}

Eq *Eq::clone() const {
  return new Eq(*this);
}

void Eq::validate() const {
  BinaryExpr::validate();

  if (lhs->is_boolean()) {
    if (!rhs->is_boolean()) {
      throw RumurError("left hand side of comparison is boolean but "
        "right hand side is not", loc);
    }
  } else if (lhs->is_arithmetic()) {
    if (!rhs->is_arithmetic()) {
      throw RumurError("left hand side of comparison is arithmetic but "
        "right hand side is not", loc);
    }
  }
  // TODO test other comparable pairs
}

const TypeExpr *Eq::type() const {
  return &Boolean;
}

void Eq::generate(std::ostream &out) const {
  out << "(" << *lhs << "==" << *rhs << ")";
}

int64_t Eq::constant_fold() const {
  return lhs->constant_fold() == rhs->constant_fold();
}

Neq &Neq::operator=(Neq other) {
  swap(*this, other);
  return *this;
}

Neq *Neq::clone() const {
  return new Neq(*this);
}

void Neq::validate() const {
  BinaryExpr::validate();

  if (lhs->is_boolean()) {
    if (!rhs->is_boolean()) {
      throw RumurError("left hand side of comparison is boolean but "
        "right hand side is not", loc);
    }
  } else if (lhs->is_arithmetic()) {
    if (!rhs->is_arithmetic()) {
      throw RumurError("left hand side of comparison is arithmetic but "
        "right hand side is not", loc);
    }
  }
  // TODO test other comparable pairs
}

const TypeExpr *Neq::type() const {
  return &Boolean;
}

void Neq::generate(std::ostream &out) const {
  out << "(" << *lhs << "!=" << *rhs << ")";
}

int64_t Neq::constant_fold() const {
  return lhs->constant_fold() != rhs->constant_fold();
}

Add &Add::operator=(Add other) {
  swap(*this, other);
  return *this;
}

Add *Add::clone() const {
  return new Add(*this);
}

void Add::validate() const {
  BinaryExpr::validate();

  expect_arithmetic(lhs);
  expect_arithmetic(rhs);
}

const TypeExpr *Add::type() const {
  return nullptr;
}

void Add::generate(std::ostream &out) const {
  out << "(" << *lhs << "+" << *rhs << ")";
}

int64_t Add::constant_fold() const {
  int64_t r;
  int64_t a = lhs->constant_fold();
  int64_t b = rhs->constant_fold();
  if (__builtin_add_overflow(a, b, &r))
    throw std::overflow_error("overflow in " + std::to_string(a) + " + "
      + std::to_string(b));
  return r;
}

Sub &Sub::operator=(Sub other) {
  swap(*this, other);
  return *this;
}

Sub *Sub::clone() const {
  return new Sub(*this);
}

void Sub::validate() const {
  BinaryExpr::validate();

  expect_arithmetic(lhs);
  expect_arithmetic(rhs);
}

const TypeExpr *Sub::type() const {
  return nullptr;
}

void Sub::generate(std::ostream &out) const {
  out << "(" << *lhs << "-" << *rhs << ")";
}

int64_t Sub::constant_fold() const {
  int64_t r;
  int64_t a = lhs->constant_fold();
  int64_t b = rhs->constant_fold();
  if (__builtin_sub_overflow(a, b, &r))
    throw std::overflow_error("overflow in " + std::to_string(a) + " - "
      + std::to_string(b));
  return r;
}

void Negative::validate() const {
  rhs->validate();
  expect_arithmetic(rhs);
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
    throw std::overflow_error("overflow in -" + std::to_string(a));
  return -a;
}

Mul &Mul::operator=(Mul other) {
  swap(*this, other);
  return *this;
}

Mul *Mul::clone() const {
  return new Mul(*this);
}

void Mul::validate() const {
  BinaryExpr::validate();

  expect_arithmetic(lhs);
  expect_arithmetic(rhs);
}

const TypeExpr *Mul::type() const {
  return nullptr;
}

void Mul::generate(std::ostream &out) const {
  out << "(" << *lhs << "*" << *rhs << ")";
}

int64_t Mul::constant_fold() const {
  int64_t r;
  int64_t a = lhs->constant_fold();
  int64_t b = rhs->constant_fold();
  if (__builtin_mul_overflow(a, b, &r))
    throw std::overflow_error("overflow in " + std::to_string(a) + " * "
      + std::to_string(b));
  return r;
}

Div &Div::operator=(Div other) {
  swap(*this, other);
  return *this;
}

Div *Div::clone() const {
  return new Div(*this);
}

void Div::validate() const {
  BinaryExpr::validate();

  expect_arithmetic(lhs);
  expect_arithmetic(rhs);
}

const TypeExpr *Div::type() const {
  return nullptr;
}

void Div::generate(std::ostream &out) const {
  out << "(" << *lhs << "/" << *rhs << ")";
}

int64_t Div::constant_fold() const {
  int64_t a = lhs->constant_fold();
  int64_t b = rhs->constant_fold();
  if (b == 0)
    throw std::out_of_range("division by 0 in " + std::to_string(a) + " / "
      + std::to_string(b));
  if (a == std::numeric_limits<int64_t>::min() && b == -1)
    throw std::overflow_error("overflow in " + std::to_string(a) + " / "
      + std::to_string(b));
  return a / b;
}

Mod &Mod::operator=(Mod other) {
  swap(*this, other);
  return *this;
}

Mod *Mod::clone() const {
  return new Mod(*this);
}

void Mod::validate() const {
  BinaryExpr::validate();

  expect_arithmetic(lhs);
  expect_arithmetic(rhs);
}

const TypeExpr *Mod::type() const {
  return nullptr;
}

void Mod::generate(std::ostream &out) const {
  out << "(" << *lhs << "%" << *rhs << ")";
}

int64_t Mod::constant_fold() const {
  int64_t a = lhs->constant_fold();
  int64_t b = rhs->constant_fold();
  if (b == 0)
    throw std::out_of_range("mod by 0 in " + std::to_string(a) + " % "
      + std::to_string(b));
  if (a == std::numeric_limits<int64_t>::min() && b == -1)
    throw std::overflow_error("overflow in " + std::to_string(a) + " % "
      + std::to_string(b));
  return a % b;
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

void ExprID::validate() const {
  // FIXME: Is this relevant? An ExprID is just referencing another expression
  // we've probably already checked.
  //value->validate();
}

const TypeExpr *ExprID::type() const {
  if (dynamic_cast<const ConstDecl*>(value) != nullptr) {
    return nullptr;
  } else if (auto t = dynamic_cast<const TypeDecl*>(value)) {
    return t->value;
  } else if (auto v = dynamic_cast<const VarDecl*>(value)) {
    return v->type;
  }
  assert(!"unreachable");
  __builtin_unreachable();
}

void ExprID::generate(std::ostream &out) const {
  out << "TODO " << id;
}

ExprID::~ExprID() {
  delete value;
}

int64_t ExprID::constant_fold() const {
  throw std::invalid_argument("symbol is not a constant");
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
  throw std::invalid_argument("field expressions are not constant");
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
  // TODO
  return nullptr;
}

void Element::generate(std::ostream &out) const {
  out << "(" << *array << "[" << *index << "])";
}

int64_t Element::constant_fold() const {
  throw std::invalid_argument("array element expressions are not constant");
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

Exists::Exists(Quantifier *quantifier_, Expr *expr_, const location &loc_):
  Expr(loc_), quantifier(quantifier_), expr(expr_) {
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

void Exists::generate(std::ostream &out) const {
  out << "({bool ru_g_TODO=false;" << *quantifier << "{if(" << *expr
    << "){ru_g_TODO=true;break;}}ru_g_TODO;})";
}

int64_t Exists::constant_fold() const {
  throw std::invalid_argument("exists expressions are not constant");
}

Forall::Forall(Quantifier *quantifier_, Expr *expr_, const location &loc_):
  Expr(loc_), quantifier(quantifier_), expr(expr_) {
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
  throw std::invalid_argument("forall expressions are not constants");
}

}
