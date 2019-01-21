#include <cassert>
#include <cstddef>
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
#include <rumur/Ptr.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include "utils.h"
#include <vector>

namespace rumur {

bool Expr::is_boolean() const {
  return type() != nullptr && *type()->resolve() == *Boolean;
}

bool Expr::is_lvalue() const {
  return false;
}

bool Expr::is_readonly() const {
  return !is_lvalue();
}

Ternary::Ternary(const Ptr<Expr> &cond_, const Ptr<Expr> &lhs_,
  const Ptr<Expr> &rhs_, const location &loc_):
  Expr(loc_), cond(cond_), lhs(lhs_), rhs(rhs_) { }

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

mpz_class Ternary::constant_fold() const {
  return (cond->constant_fold() != 0) ? lhs->constant_fold() : rhs->constant_fold();
}

void Ternary::validate() const {
  if (!cond->is_boolean())
    throw Error("ternary condition is not a boolean", cond->loc);
}

std::string Ternary::to_string() const {
  return "(" + cond->to_string() + " ? " + lhs->to_string() + " : "
    + rhs->to_string() + ")";
}

BinaryExpr::BinaryExpr(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
  const location &loc_):
  Expr(loc_), lhs(lhs_), rhs(rhs_) { }

bool BinaryExpr::constant() const {
  return lhs->constant() && rhs->constant();
}

void BooleanBinaryExpr::validate() const {
  if (!lhs->is_boolean())
    throw Error("left hand side of expression is not a boolean", lhs->loc);

  if (!rhs->is_boolean())
    throw Error("right hand side of expression is not a boolean",
      rhs->loc);
}

Implication *Implication::clone() const {
  return new Implication(*this);
}

const TypeExpr *Implication::type() const {
  return Boolean.get();
}

mpz_class Implication::constant_fold() const {
  return lhs->constant_fold() == 0 || rhs->constant_fold() != 0;
}

bool Implication::operator==(const Node &other) const {
  auto o = dynamic_cast<const Implication*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

std::string Implication::to_string() const {
  return "(" + lhs->to_string() + " -> " + rhs->to_string() + ")";
}

Or *Or::clone() const {
  return new Or(*this);
}

const TypeExpr *Or::type() const {
  return Boolean.get();
}

mpz_class Or::constant_fold() const {
  return lhs->constant_fold() != 0 || rhs->constant_fold() != 0;
}

bool Or::operator==(const Node &other) const {
  auto o = dynamic_cast<const Or*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

std::string Or::to_string() const {
  return "(" + lhs->to_string() + " | " + rhs->to_string() + ")";
}

And *And::clone() const {
  return new And(*this);
}

const TypeExpr *And::type() const {
  return Boolean.get();
}

mpz_class And::constant_fold() const {
  return lhs->constant_fold() != 0 && rhs->constant_fold() != 0;
}

bool And::operator==(const Node &other) const {
  auto o = dynamic_cast<const And*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

std::string And::to_string() const {
  return "(" + lhs->to_string() + " & " + rhs->to_string() + ")";
}

UnaryExpr::UnaryExpr(const Ptr<Expr> &rhs_, const location &loc_):
  Expr(loc_), rhs(rhs_) {
}

bool UnaryExpr::constant() const {
  return rhs->constant();
}

Not *Not::clone() const {
  return new Not(*this);
}

const TypeExpr *Not::type() const {
  return Boolean.get();
}

mpz_class Not::constant_fold() const {
  return rhs->constant_fold() == 0;
}

bool Not::operator==(const Node &other) const {
  auto o = dynamic_cast<const Not*>(&other);
  return o != nullptr && *rhs == *o->rhs;
}

void Not::validate() const {
  if (!rhs->is_boolean())
    throw Error("argument to ! is not a boolean", rhs->loc);
}

std::string Not::to_string() const {
  return "(!" + rhs->to_string() + ")";
}

static bool comparable(const Expr &lhs, const Expr &rhs) {

  if (lhs.type() == nullptr) {
    // LHS is a numeric literal

    if (rhs.type() == nullptr)
      return true;

    const TypeExpr *t = rhs.type()->resolve();

    if (isa<Range>(t))
      return true;

    return false;
  }
  
  if (rhs.type() == nullptr) {
    // RHS is a numeric literal

    const TypeExpr *t = lhs.type()->resolve();

    if (isa<Range>(t))
      return true;

    return false;
  }

  const TypeExpr *t1 = lhs.type()->resolve();
  const TypeExpr *t2 = rhs.type()->resolve();

  if (isa<Range>(t1)) {
    if (isa<Range>(t2))
      return true;
  }

  return false;
}

void ComparisonBinaryExpr::validate() const {
  if (!comparable(*lhs, *rhs))
    throw Error("expressions are not comparable", loc);
}

Lt *Lt::clone() const {
  return new Lt(*this);
}

const TypeExpr *Lt::type() const {
  return Boolean.get();
}

mpz_class Lt::constant_fold() const {
  return lhs->constant_fold() < rhs->constant_fold();
}

bool Lt::operator==(const Node &other) const {
  auto o = dynamic_cast<const Lt*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

std::string Lt::to_string() const {
  return "(" + lhs->to_string() + " < " + rhs->to_string() + ")";
}

Leq *Leq::clone() const {
  return new Leq(*this);
}

const TypeExpr *Leq::type() const {
  return Boolean.get();
}

mpz_class Leq::constant_fold() const {
  return lhs->constant_fold() <= rhs->constant_fold();
}

bool Leq::operator==(const Node &other) const {
  auto o = dynamic_cast<const Leq*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

std::string Leq::to_string() const {
  return "(" + lhs->to_string() + " <= " + rhs->to_string() + ")";
}

Gt *Gt::clone() const {
  return new Gt(*this);
}

const TypeExpr *Gt::type() const {
  return Boolean.get();
}

mpz_class Gt::constant_fold() const {
  return lhs->constant_fold() > rhs->constant_fold();
}

bool Gt::operator==(const Node &other) const {
  auto o = dynamic_cast<const Gt*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

std::string Gt::to_string() const {
  return "(" + lhs->to_string() + " > " + rhs->to_string() + ")";
}

Geq *Geq::clone() const {
  return new Geq(*this);
}

const TypeExpr *Geq::type() const {
  return Boolean.get();
}

mpz_class Geq::constant_fold() const {
  return lhs->constant_fold() >= rhs->constant_fold();
}

bool Geq::operator==(const Node &other) const {
  auto o = dynamic_cast<const Geq*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

std::string Geq::to_string() const {
  return "(" + lhs->to_string() + " >= " + rhs->to_string() + ")";
}

void EquatableBinaryExpr::validate() const {
  if (!types_equatable(lhs->type(), rhs->type()))
    throw Error("expressions are not comparable", loc);
}

Eq *Eq::clone() const {
  return new Eq(*this);
}

const TypeExpr *Eq::type() const {
  return Boolean.get();
}

mpz_class Eq::constant_fold() const {
  return lhs->constant_fold() == rhs->constant_fold();
}

bool Eq::operator==(const Node &other) const {
  auto o = dynamic_cast<const Eq*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

std::string Eq::to_string() const {
  return "(" + lhs->to_string() + " = " + rhs->to_string() + ")";
}

Neq *Neq::clone() const {
  return new Neq(*this);
}

const TypeExpr *Neq::type() const {
  return Boolean.get();
}

mpz_class Neq::constant_fold() const {
  return lhs->constant_fold() != rhs->constant_fold();
}

bool Neq::operator==(const Node &other) const {
  auto o = dynamic_cast<const Neq*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

std::string Neq::to_string() const {
  return "(" + lhs->to_string() + " != " + rhs->to_string() + ")";
}

static bool arithmetic(const Expr &lhs, const Expr &rhs) {

  if (lhs.type() == nullptr) {
    // LHS is a numeric literal

    if (rhs.type() == nullptr)
      return true;

    const TypeExpr *t = rhs.type()->resolve();

    if (isa<Range>(t))
      return true;

    return false;
  }
  
  if (rhs.type() == nullptr) {
    // RHS is a numeric literal

    const TypeExpr *t = lhs.type()->resolve();

    if (isa<Range>(t))
      return true;

    return false;
  }

  const TypeExpr *t1 = lhs.type()->resolve();
  const TypeExpr *t2 = rhs.type()->resolve();

  if (isa<Range>(t1) && isa<Range>(t2))
    return true;

  return false;
}


void ArithmeticBinaryExpr::validate() const {
  if (!arithmetic(*lhs, *rhs))
    throw Error("expressions are incompatible in arithmetic expression",
      loc);
}

Add *Add::clone() const {
  return new Add(*this);
}

const TypeExpr *Add::type() const {
  return nullptr;
}

mpz_class Add::constant_fold() const {
  return lhs->constant_fold() + rhs->constant_fold();
}

bool Add::operator==(const Node &other) const {
  auto o = dynamic_cast<const Add*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

std::string Add::to_string() const {
  return "(" + lhs->to_string() + " + " + rhs->to_string() + ")";
}

Sub *Sub::clone() const {
  return new Sub(*this);
}

const TypeExpr *Sub::type() const {
  return nullptr;
}

mpz_class Sub::constant_fold() const {
  return lhs->constant_fold() - rhs->constant_fold();
}

bool Sub::operator==(const Node &other) const {
  auto o = dynamic_cast<const Sub*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

std::string Sub::to_string() const {
  return "(" + lhs->to_string() + " - " + rhs->to_string() + ")";
}

void Negative::validate() const {
  if (rhs->type() != nullptr && isa<Range>(rhs->type()->resolve()))
    throw Error("expression cannot be negated", rhs->loc);
}

Negative *Negative::clone() const {
  return new Negative(*this);
}

const TypeExpr *Negative::type() const {
  return rhs->type();
}

mpz_class Negative::constant_fold() const {
  return -rhs->constant_fold();
}

bool Negative::operator==(const Node &other) const {
  auto o = dynamic_cast<const Negative*>(&other);
  return o != nullptr && *rhs == *o->rhs;
}

std::string Negative::to_string() const {
  return "(-" + rhs->to_string() + ")";
}

Mul *Mul::clone() const {
  return new Mul(*this);
}

const TypeExpr *Mul::type() const {
  return nullptr;
}

mpz_class Mul::constant_fold() const {
  return lhs->constant_fold() * rhs->constant_fold();
}

bool Mul::operator==(const Node &other) const {
  auto o = dynamic_cast<const Mul*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

std::string Mul::to_string() const {
  return "(" + lhs->to_string() + " * " + rhs->to_string() + ")";
}

Div *Div::clone() const {
  return new Div(*this);
}

const TypeExpr *Div::type() const {
  return nullptr;
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

std::string Div::to_string() const {
  return "(" + lhs->to_string() + " / " + rhs->to_string() + ")";
}

Mod *Mod::clone() const {
  return new Mod(*this);
}

const TypeExpr *Mod::type() const {
  return nullptr;
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

std::string Mod::to_string() const {
  return "(" + lhs->to_string() + " % " + rhs->to_string() + ")";
}

ExprID::ExprID(const std::string &id_, const Ptr<ExprDecl> &value_,
  const location &loc_):
  Expr(loc_), id(id_), value(value_) {
}

ExprID *ExprID::clone() const {
  return new ExprID(*this);
}

bool ExprID::constant() const {
  return isa<ConstDecl>(value);
}

const TypeExpr *ExprID::type() const {
  if (value == nullptr)
    throw Error("symbol \"" + id + "\" in expression is unresolved", loc);

  return value->get_type();
}

mpz_class ExprID::constant_fold() const {
  if (auto c = dynamic_cast<const ConstDecl*>(value.get()))
    return c->value->constant_fold();
  throw Error("symbol \"" + id + "\" is not a constant", loc);
}

bool ExprID::operator==(const Node &other) const {
  auto o = dynamic_cast<const ExprID*>(&other);
  if (o == nullptr)
    return false;
  if (id != o->id)
    return false;
  if (value == nullptr) {
    if (o->value != nullptr)
      return false;
  } else if (o->value == nullptr) {
    return false;
  } else if (*value != *o->value) {
    return false;
  }
  return true;
}

void ExprID::validate() const {
  if (value == nullptr)
    throw Error("unresolved expression \"" + id + "\"", loc);
}

bool ExprID::is_lvalue() const {
  if (value == nullptr)
    throw Error("unresolved expression \"" + id + "\"", loc);

  return value->is_lvalue();
}

bool ExprID::is_readonly() const {
  return value->is_readonly();
}

std::string ExprID::to_string() const {
  return id;
}

Field::Field(const Ptr<Expr> &record_, const std::string &field_,
  const location &loc_):
  Expr(loc_), record(record_), field(field_) {
}

Field *Field::clone() const {
  return new Field(*this);
}

bool Field::constant() const {
  return false;
}

const TypeExpr *Field::type() const {

  const TypeExpr *root = record->type();
  assert(root != nullptr && "invalid left hand side of field expression");
  const TypeExpr *resolved = root->resolve();
  assert(resolved != nullptr && "invalid left hand side of field expression");

  auto r = dynamic_cast<const Record*>(resolved);
  assert(r != nullptr && "invalid left hand side of field expression");

  for (const Ptr<VarDecl> &f : r->fields) {
    if (f->name == field)
      return f->type.get();
  }

  throw Error("no field named \"" + field + "\" in record", loc);
}

mpz_class Field::constant_fold() const {
  throw Error("field expression used in constant", loc);
}

bool Field::operator==(const Node &other) const {
  auto o = dynamic_cast<const Field*>(&other);
  return o != nullptr && *record == *o->record && field == o->field;
}

void Field::validate() const {

  const TypeExpr *root = record->type();
  if (root != nullptr)
    root = root->resolve();

  if (!isa<Record>(root))
    throw Error("left hand side of field expression is not a record", loc);

  auto r = dynamic_cast<const Record*>(root);
  assert(r != nullptr && "logic error in Field::validate");

  for (const Ptr<VarDecl> &f : r->fields) {
    if (f->name == field)
      return;
  }

  throw Error("no field named \"" + field + "\" in record", loc);
}

bool Field::is_lvalue() const {
  return record->is_lvalue();
}

bool Field::is_readonly() const {
  return record->is_readonly();
}

std::string Field::to_string() const {
  return record->to_string() + "." + field;
}

Element::Element(const Ptr<Expr> &array_, const Ptr<Expr> &index_,
  const location &loc_):
  Expr(loc_), array(array_), index(index_) {
}

Element *Element::clone() const {
  return new Element(*this);
}

bool Element::constant() const {
  return false;
}

const TypeExpr *Element::type() const {
  const TypeExpr *t = array->type()->resolve();
  const Array *a = dynamic_cast<const Array*>(t);
  assert(a != nullptr &&
    "array reference based on something that is not an array");
  return a->element_type.get();
}

mpz_class Element::constant_fold() const {
  throw Error("array element used in constant", loc);
}

bool Element::operator==(const Node &other) const {
  auto o = dynamic_cast<const Element*>(&other);
  return o != nullptr && *array == *o->array && *index == *o->index;
}

void Element::validate() const {

  const TypeExpr *t = array->type();
  if (t != nullptr)
    t = t->resolve();

  if (!isa<Array>(t))
    throw Error("array index on an expression that is not an array", loc);

  auto a = dynamic_cast<const Array*>(t);
  assert(a != nullptr && "logic error in Element::validate");

  const TypeExpr *e = index->type();
  if (e != nullptr)
    e = e->resolve();

  const TypeExpr *index_type = a->index_type->resolve();

  if (isa<Range>(index_type)) {
    if (e != nullptr && !isa<Range>(e))
      throw Error("array indexed using an expression of incorrect type", loc);

  } else {
    if (e == nullptr || *index_type != *e)
      throw Error("array indexed using an expression of incorrect type", loc);

  }
}

bool Element::is_lvalue() const {
  return array->is_lvalue();
}

bool Element::is_readonly() const {
  return array->is_readonly();
}

std::string Element::to_string() const {
  return array->to_string() + "[" + index->to_string() + "]";
}

FunctionCall::FunctionCall(const std::string &name_,
  const std::vector<Ptr<Expr>> &arguments_, const location &loc_):
  Expr(loc_), name(name_), arguments(arguments_) { }

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
  if (function == nullptr)
    throw Error("unresolved function call \"" + name + "\"", loc);
  return function->return_type.get();
}

mpz_class FunctionCall::constant_fold() const {
  // See FunctionCall::constant() regarding conservatism here.
  throw Error("function call used in a constant", loc);
}

bool FunctionCall::operator==(const Node &other) const {
  auto o = dynamic_cast<const FunctionCall*>(&other);
  if (o == nullptr)
    return false;
  if (name != o->name)
    return false;
  if (function == nullptr) {
    if (o->function != nullptr)
      return false;
  } else if (o->function == nullptr) {
    return false;
  } else if (*function != *o->function) {
    return false;
  }
  if (!vector_eq(arguments, o->arguments))
    return false;
  return true;
}

void FunctionCall::validate() const {
  if (function == nullptr)
    throw Error("unknown function call \"" + name + "\"", loc);

  if (arguments.size() != function->parameters.size())
    throw Error("incorrect number of parameters passed to function", loc);

  auto it = arguments.begin();
  for (const Ptr<VarDecl> &v : function->parameters) {

    assert(it != arguments.end() && "mismatch in size of parameter list and "
      "function arguments list");

    if ((*it)->is_readonly() && !v->is_readonly())
      throw Error("function call passes a read-only value as a var parameter",
        (*it)->loc);

    const TypeExpr *arg_type = (*it)->type();
    if (arg_type != nullptr)
      arg_type = arg_type->resolve();

    const TypeExpr *param_type = v->get_type();
    assert(param_type != nullptr && "function parameter has no type");
    param_type = param_type->resolve();

    if (arg_type == nullptr) {
      if (!isa<Range>(param_type))
        throw Error("function call contains parameter of incorrect type",
          (*it)->loc);

    } else if (isa<Range>(arg_type)) {
      if (!isa<Range>(param_type))
        throw Error("function call contains parameter of incorrect type",
          (*it)->loc);

      if (*arg_type != *param_type && !v->is_readonly())
        throw Error("range types of function call argument and var parameter "
          "differ", (*it)->loc);

    } else if (*arg_type != *param_type) {
      throw Error("function call contains parameter of incorrect type",
        (*it)->loc);
    }

    it++;
  }
}

std::string FunctionCall::to_string() const {
  std::string s = name + "(";
  bool first = true;
  for (const Ptr<Expr> &arg : arguments) {
    if (!first)
      s += ", ";
    s += arg->to_string();
  }
  s += ")";
  return s;
}

Quantifier::Quantifier(const std::string &name_, const Ptr<TypeExpr> &type_,
  const location &loc_):
  Node(loc_), name(name_), type(type_) { }

Quantifier::Quantifier(const std::string &name_, const Ptr<Expr> &from_,
  const Ptr<Expr> &to_, const location &loc_):
  Quantifier(name_, from_, to_, nullptr, loc_) { }

Quantifier::Quantifier(const std::string &name_, const Ptr<Expr> &from_,
  const Ptr<Expr> &to_, const Ptr<Expr> &step_, const location &loc_):
  Node(loc_), name(name_), from(from_), to(to_), step(step_) { }

Quantifier *Quantifier::clone() const {
  return new Quantifier(*this);
}

bool Quantifier::operator==(const Node &other) const {
  auto o = dynamic_cast<const Quantifier*>(&other);
  if (o == nullptr)
    return false;
  if (name != o->name)
    return false;
  if (type == nullptr) {
    if (o->type != nullptr)
      return false;
  } else if (o->type == nullptr) {
    return false;
  } else if (*type != *o->type) {
    return false;
  }
  if (from == nullptr) {
    if (o->from != nullptr)
      return false;
  } else if (o->from == nullptr) {
    return false;
  } else if (*from != *o->from) {
    return false;
  }
  if (to == nullptr) {
    if (o->to != nullptr)
      return false;
  } else if (o->to == nullptr) {
    return false;
  } else if (*to != *o->to) {
    return false;
  }
  if (step == nullptr) {
    if (o->step != nullptr)
      return false;
  } else if (o->step == nullptr) {
    return false;
  } else if (*step != *o->step) {
    return false;
  }
  return true;
}

std::string Quantifier::to_string() const {
  if (type == nullptr) {
    std::string s = name + " from " + from->to_string() + " to "
      + to->to_string();
    if (step != nullptr)
      s += " by " + step->to_string();
    return s;
  }

  return name + " : " + type->to_string();
}

bool Quantifier::constant() const {

  if (type != nullptr) {
    assert(type->is_simple() && "complex type used in quantifier");
    if (!type->constant())
      return false;
  }

  if (from != nullptr && !from->constant())
    return false;

  if (to != nullptr && !to->constant())
    return false;

  if (step != nullptr && !step->constant())
    return false;

  return true;
}

Exists::Exists(const Quantifier &quantifier_, const Ptr<Expr> &expr_,
  const location &loc_):
  Expr(loc_), quantifier(quantifier_), expr(expr_) { }

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
  return o != nullptr && quantifier == o->quantifier && *expr == *o->expr;
}

mpz_class Exists::constant_fold() const {
  throw Error("exists expression used in constant", loc);
}

void Exists::validate() const {
  if (!expr->is_boolean())
    throw Error("expression in exists is not boolean", expr->loc);
}

std::string Exists::to_string() const {
  return "exists " + quantifier.to_string() + " do " + expr->to_string()
    + " endexists";
}

Forall::Forall(const Quantifier &quantifier_, const Ptr<Expr> &expr_,
  const location &loc_):
  Expr(loc_), quantifier(quantifier_), expr(expr_) { }

Forall *Forall::clone() const {
  return new Forall(*this);
}

bool Forall::constant() const {
  return expr->constant();
}

const TypeExpr *Forall::type() const {
  return Boolean.get();
}

mpz_class Forall::constant_fold() const {
  throw Error("forall expression used in constant", loc);
}

bool Forall::operator==(const Node &other) const {
  auto o = dynamic_cast<const Forall*>(&other);
  return o != nullptr && quantifier == o->quantifier && *expr == *o->expr;
}

void Forall::validate() const {
  if (!expr->is_boolean())
    throw Error("expression in forall is not boolean", expr->loc);
}

std::string Forall::to_string() const {
  return "forall " + quantifier.to_string() + " do " + expr->to_string()
    + " endforall";
}

IsUndefined::IsUndefined(const Ptr<Expr> &expr_, const location &loc_):
  Expr(loc_), expr(expr_) { }

IsUndefined *IsUndefined::clone() const {
  return new IsUndefined(*this);
}

bool IsUndefined::constant() const {
  return false;
}

const TypeExpr *IsUndefined::type() const {
  return Boolean.get();
}

mpz_class IsUndefined::constant_fold() const {
  throw Error("isundefined used in constant", loc);
}

bool IsUndefined::operator==(const Node &other) const {
  auto o = dynamic_cast<const IsUndefined*>(&other);
  if (o == nullptr)
    return false;
  if (*expr != *o->expr)
    return false;
  return true;
}

void IsUndefined::validate() const {

  if (!expr->is_lvalue())
    throw Error("non-lvalue expression cannot be used in isundefined",
      expr->loc);

  const TypeExpr *t = expr->type();
  if (t != nullptr && !t->is_simple())
    throw Error("complex type used in isundefined", expr->loc);

}

std::string IsUndefined::to_string() const {
  return "isundefined(" + expr->to_string() + ")";
}

}
