#include "../../common/isa.h"
#include "location.hh"
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <iostream>
#include <limits>
#include <memory>
#include <rumur/Boolean.h>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Function.h>
#include <rumur/Ptr.h>
#include <rumur/TypeExpr.h>
#include <rumur/except.h>
#include <rumur/traverse.h>
#include <string>
#include <utility>
#include <vector>

namespace rumur {

Expr::Expr(const location &loc_) : Node(loc_) {}

bool Expr::is_boolean() const { return type()->resolve()->is_boolean(); }

bool Expr::is_lvalue() const { return false; }

bool Expr::is_readonly() const { return !is_lvalue(); }

bool Expr::is_literal_true() const { return false; }

bool Expr::is_literal_false() const { return false; }

Ternary::Ternary(const Ptr<Expr> &cond_, const Ptr<Expr> &lhs_,
                 const Ptr<Expr> &rhs_, const location &loc_)
    : Expr(loc_), cond(cond_), lhs(lhs_), rhs(rhs_) {}

Ternary *Ternary::clone() const { return new Ternary(*this); }

void Ternary::visit(BaseTraversal &visitor) {
  return visitor.visit_ternary(*this);
}

void Ternary::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_ternary(*this);
}

bool Ternary::constant() const {
  return cond->constant() && lhs->constant() && rhs->constant();
}

Ptr<TypeExpr> Ternary::type() const {
  // TODO: assert lhs and rhs are compatible types.
  return lhs->type();
}

mpz_class Ternary::constant_fold() const {
  return cond->constant_fold() != 0 ? lhs->constant_fold()
                                    : rhs->constant_fold();
}

void Ternary::validate() const {
  if (!cond->is_boolean())
    throw Error("ternary condition is not a boolean", cond->loc);
}

std::string Ternary::to_string() const {
  return "(" + cond->to_string() + " ? " + lhs->to_string() + " : " +
         rhs->to_string() + ")";
}

bool Ternary::is_pure() const {
  return cond->is_pure() && lhs->is_pure() && rhs->is_pure();
}

BinaryExpr::BinaryExpr(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
                       const location &loc_)
    : Expr(loc_), lhs(lhs_), rhs(rhs_) {}

bool BinaryExpr::constant() const { return lhs->constant() && rhs->constant(); }

bool BinaryExpr::is_pure() const { return lhs->is_pure() && rhs->is_pure(); }

BooleanBinaryExpr::BooleanBinaryExpr(const Ptr<Expr> &lhs_,
                                     const Ptr<Expr> &rhs_,
                                     const location &loc_)
    : BinaryExpr(lhs_, rhs_, loc_) {}

Ptr<TypeExpr> BooleanBinaryExpr::type() const { return Boolean; }

void BooleanBinaryExpr::validate() const {
  if (!lhs->is_boolean())
    throw Error("left hand side of expression is not a boolean", lhs->loc);

  if (!rhs->is_boolean())
    throw Error("right hand side of expression is not a boolean", rhs->loc);
}

Implication::Implication(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
                         const location &loc_)
    : BooleanBinaryExpr(lhs_, rhs_, loc_) {}

Implication *Implication::clone() const { return new Implication(*this); }

void Implication::visit(BaseTraversal &visitor) {
  return visitor.visit_implication(*this);
}

void Implication::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_implication(*this);
}

mpz_class Implication::constant_fold() const {
  return lhs->constant_fold() == 0 || rhs->constant_fold() != 0;
}

std::string Implication::to_string() const {
  return "(" + lhs->to_string() + " -> " + rhs->to_string() + ")";
}

Or::Or(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : BooleanBinaryExpr(lhs_, rhs_, loc_) {}

Or *Or::clone() const { return new Or(*this); }

void Or::visit(BaseTraversal &visitor) { return visitor.visit_or(*this); }

void Or::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_or(*this);
}

mpz_class Or::constant_fold() const {
  return lhs->constant_fold() != 0 || rhs->constant_fold() != 0;
}

std::string Or::to_string() const {
  return "(" + lhs->to_string() + " | " + rhs->to_string() + ")";
}

And::And(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : BooleanBinaryExpr(lhs_, rhs_, loc_) {}

And *And::clone() const { return new And(*this); }

void And::visit(BaseTraversal &visitor) { return visitor.visit_and(*this); }

void And::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_and(*this);
}

mpz_class And::constant_fold() const {
  return lhs->constant_fold() != 0 && rhs->constant_fold() != 0;
}

std::string And::to_string() const {
  return "(" + lhs->to_string() + " & " + rhs->to_string() + ")";
}

AmbiguousAmp::AmbiguousAmp(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
                           const location &loc_)
    : BinaryExpr(lhs_, rhs_, loc_) {}

AmbiguousAmp *AmbiguousAmp::clone() const { return new AmbiguousAmp(*this); }

void AmbiguousAmp::visit(BaseTraversal &visitor) {
  return visitor.visit_ambiguousamp(*this);
}

void AmbiguousAmp::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_ambiguousamp(*this);
}

Ptr<TypeExpr> AmbiguousAmp::type() const {
  // we cannot retrieve the type of this expression because it has no certain
  // type until we decide whether it is a logical AND or bitwise AND
  throw Error("cannot retrieve the type of an unresolved '&' expression", loc);
}

mpz_class AmbiguousAmp::constant_fold() const {
  // we cannot constant fold this if we do not yet know whether it is a logical
  // AND or a bitwise AND
  throw Error("cannot constant fold an unresolved '&' expression", loc);
}

std::string AmbiguousAmp::to_string() const {
  return "(" + lhs->to_string() + " & " + rhs->to_string() + ")";
}

AmbiguousPipe::AmbiguousPipe(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
                             const location &loc_)
    : BinaryExpr(lhs_, rhs_, loc_) {}

AmbiguousPipe *AmbiguousPipe::clone() const { return new AmbiguousPipe(*this); }

void AmbiguousPipe::visit(BaseTraversal &visitor) {
  return visitor.visit_ambiguouspipe(*this);
}

void AmbiguousPipe::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_ambiguouspipe(*this);
}

Ptr<TypeExpr> AmbiguousPipe::type() const {
  // we cannot retrieve the type of this expression because it has no certain
  // type until we decide whether it is a logical OR or bitwise OR
  throw Error("cannot retrieve the type of an unresolved '|' expression", loc);
}

mpz_class AmbiguousPipe::constant_fold() const {
  // we cannot constant fold this if we do not yet know whether it is a logical
  // OR or a bitwise OR
  throw Error("cannot constant fold an unresolved '|' expression", loc);
}

std::string AmbiguousPipe::to_string() const {
  return "(" + lhs->to_string() + " | " + rhs->to_string() + ")";
}

UnaryExpr::UnaryExpr(const Ptr<Expr> &rhs_, const location &loc_)
    : Expr(loc_), rhs(rhs_) {}

bool UnaryExpr::constant() const { return rhs->constant(); }

bool UnaryExpr::is_pure() const { return rhs->is_pure(); }

Not::Not(const Ptr<Expr> &rhs_, const location &loc_) : UnaryExpr(rhs_, loc_) {}

Not *Not::clone() const { return new Not(*this); }

void Not::visit(BaseTraversal &visitor) { return visitor.visit_not(*this); }

void Not::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_not(*this);
}

Ptr<TypeExpr> Not::type() const { return Boolean; }

mpz_class Not::constant_fold() const { return rhs->constant_fold() == 0; }

void Not::validate() const {
  if (!rhs->is_boolean())
    throw Error("argument to ! is not a boolean", rhs->loc);
}

std::string Not::to_string() const { return "(!" + rhs->to_string() + ")"; }

static bool comparable(const Expr &lhs, const Expr &rhs) {

  const Ptr<TypeExpr> t1 = lhs.type()->resolve();
  const Ptr<TypeExpr> t2 = rhs.type()->resolve();

  if (isa<Range>(t1)) {
    if (isa<Range>(t2))
      return true;
  }

  return false;
}

ComparisonBinaryExpr::ComparisonBinaryExpr(const Ptr<Expr> &lhs_,
                                           const Ptr<Expr> &rhs_,
                                           const location &loc_)
    : BinaryExpr(lhs_, rhs_, loc_) {}

void ComparisonBinaryExpr::validate() const {
  if (!comparable(*lhs, *rhs))
    throw Error("expressions are not comparable", loc);
}

Lt::Lt(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : ComparisonBinaryExpr(lhs_, rhs_, loc_) {}

Lt *Lt::clone() const { return new Lt(*this); }

void Lt::visit(BaseTraversal &visitor) { return visitor.visit_lt(*this); }

void Lt::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_lt(*this);
}

Ptr<TypeExpr> Lt::type() const { return Boolean; }

mpz_class Lt::constant_fold() const {
  return lhs->constant_fold() < rhs->constant_fold();
}

std::string Lt::to_string() const {
  return "(" + lhs->to_string() + " < " + rhs->to_string() + ")";
}

Leq::Leq(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : ComparisonBinaryExpr(lhs_, rhs_, loc_) {}

Leq *Leq::clone() const { return new Leq(*this); }

void Leq::visit(BaseTraversal &visitor) { return visitor.visit_leq(*this); }

void Leq::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_leq(*this);
}

Ptr<TypeExpr> Leq::type() const { return Boolean; }

mpz_class Leq::constant_fold() const {
  return lhs->constant_fold() <= rhs->constant_fold();
}

std::string Leq::to_string() const {
  return "(" + lhs->to_string() + " <= " + rhs->to_string() + ")";
}

Gt::Gt(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : ComparisonBinaryExpr(lhs_, rhs_, loc_) {}

Gt *Gt::clone() const { return new Gt(*this); }

void Gt::visit(BaseTraversal &visitor) { return visitor.visit_gt(*this); }

void Gt::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_gt(*this);
}

Ptr<TypeExpr> Gt::type() const { return Boolean; }

mpz_class Gt::constant_fold() const {
  return lhs->constant_fold() > rhs->constant_fold();
}

std::string Gt::to_string() const {
  return "(" + lhs->to_string() + " > " + rhs->to_string() + ")";
}

Geq::Geq(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : ComparisonBinaryExpr(lhs_, rhs_, loc_) {}

Geq *Geq::clone() const { return new Geq(*this); }

void Geq::visit(BaseTraversal &visitor) { return visitor.visit_geq(*this); }

void Geq::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_geq(*this);
}

Ptr<TypeExpr> Geq::type() const { return Boolean; }

mpz_class Geq::constant_fold() const {
  return lhs->constant_fold() >= rhs->constant_fold();
}

std::string Geq::to_string() const {
  return "(" + lhs->to_string() + " >= " + rhs->to_string() + ")";
}

EquatableBinaryExpr::EquatableBinaryExpr(const Ptr<Expr> &lhs_,
                                         const Ptr<Expr> &rhs_,
                                         const location &loc_)
    : BinaryExpr(lhs_, rhs_, loc_) {}

void EquatableBinaryExpr::validate() const {
  if (!lhs->type()->coerces_to(*rhs->type()))
    throw Error("expressions are not comparable", loc);
}

Eq::Eq(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : EquatableBinaryExpr(lhs_, rhs_, loc_) {}

Eq *Eq::clone() const { return new Eq(*this); }

void Eq::visit(BaseTraversal &visitor) { return visitor.visit_eq(*this); }

void Eq::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_eq(*this);
}

Ptr<TypeExpr> Eq::type() const { return Boolean; }

mpz_class Eq::constant_fold() const {
  return lhs->constant_fold() == rhs->constant_fold();
}

std::string Eq::to_string() const {
  return "(" + lhs->to_string() + " = " + rhs->to_string() + ")";
}

Neq::Neq(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : EquatableBinaryExpr(lhs_, rhs_, loc_) {}

Neq *Neq::clone() const { return new Neq(*this); }

void Neq::visit(BaseTraversal &visitor) { return visitor.visit_neq(*this); }

void Neq::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_neq(*this);
}

Ptr<TypeExpr> Neq::type() const { return Boolean; }

mpz_class Neq::constant_fold() const {
  return lhs->constant_fold() != rhs->constant_fold();
}

std::string Neq::to_string() const {
  return "(" + lhs->to_string() + " != " + rhs->to_string() + ")";
}

static bool arithmetic(const Expr &lhs, const Expr &rhs) {

  const Ptr<TypeExpr> t1 = lhs.type()->resolve();
  const Ptr<TypeExpr> t2 = rhs.type()->resolve();

  if (isa<Range>(t1) && isa<Range>(t2))
    return true;

  return false;
}

ArithmeticBinaryExpr::ArithmeticBinaryExpr(const Ptr<Expr> &lhs_,
                                           const Ptr<Expr> &rhs_,
                                           const location &loc_)
    : BinaryExpr(lhs_, rhs_, loc_) {}

void ArithmeticBinaryExpr::validate() const {
  if (!arithmetic(*lhs, *rhs))
    throw Error("expressions are incompatible in arithmetic expression", loc);
}

Ptr<TypeExpr> ArithmeticBinaryExpr::type() const {
  return Ptr<Range>::make(nullptr, nullptr, location());
}

Add::Add(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : ArithmeticBinaryExpr(lhs_, rhs_, loc_) {}

Add *Add::clone() const { return new Add(*this); }

void Add::visit(BaseTraversal &visitor) { return visitor.visit_add(*this); }

void Add::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_add(*this);
}

mpz_class Add::constant_fold() const {
  return lhs->constant_fold() + rhs->constant_fold();
}

std::string Add::to_string() const {
  return "(" + lhs->to_string() + " + " + rhs->to_string() + ")";
}

Sub::Sub(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : ArithmeticBinaryExpr(lhs_, rhs_, loc_) {}

Sub *Sub::clone() const { return new Sub(*this); }

void Sub::visit(BaseTraversal &visitor) { return visitor.visit_sub(*this); }

void Sub::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_sub(*this);
}

mpz_class Sub::constant_fold() const {
  return lhs->constant_fold() - rhs->constant_fold();
}

std::string Sub::to_string() const {
  return "(" + lhs->to_string() + " - " + rhs->to_string() + ")";
}

Negative::Negative(const Ptr<Expr> &rhs_, const location &loc_)
    : UnaryExpr(rhs_, loc_) {}

void Negative::validate() const {
  if (!isa<Range>(rhs->type()->resolve()))
    throw Error("expression cannot be negated", rhs->loc);
}

Negative *Negative::clone() const { return new Negative(*this); }

void Negative::visit(BaseTraversal &visitor) {
  return visitor.visit_negative(*this);
}

void Negative::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_negative(*this);
}

Ptr<TypeExpr> Negative::type() const {
  return Ptr<Range>::make(nullptr, nullptr, location());
}

mpz_class Negative::constant_fold() const { return -rhs->constant_fold(); }

std::string Negative::to_string() const {
  return "(-" + rhs->to_string() + ")";
}

Bnot::Bnot(const Ptr<Expr> &rhs_, const location &loc_)
    : UnaryExpr(rhs_, loc_) {}

void Bnot::validate() const {
  if (!isa<Range>(rhs->type()->resolve()))
    throw Error("expression cannot be bitwise NOTed", rhs->loc);
}

Bnot *Bnot::clone() const { return new Bnot(*this); }

void Bnot::visit(BaseTraversal &visitor) { return visitor.visit_bnot(*this); }

void Bnot::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_bnot(*this);
}

Ptr<TypeExpr> Bnot::type() const {
  return Ptr<Range>::make(nullptr, nullptr, location());
}

mpz_class Bnot::constant_fold() const { return ~rhs->constant_fold(); }

std::string Bnot::to_string() const { return "(~" + rhs->to_string() + ")"; }

Mul::Mul(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : ArithmeticBinaryExpr(lhs_, rhs_, loc_) {}

Mul *Mul::clone() const { return new Mul(*this); }

void Mul::visit(BaseTraversal &visitor) { return visitor.visit_mul(*this); }

void Mul::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_mul(*this);
}

mpz_class Mul::constant_fold() const {
  return lhs->constant_fold() * rhs->constant_fold();
}

std::string Mul::to_string() const {
  return "(" + lhs->to_string() + " * " + rhs->to_string() + ")";
}

Div::Div(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : ArithmeticBinaryExpr(lhs_, rhs_, loc_) {}

Div *Div::clone() const { return new Div(*this); }

void Div::visit(BaseTraversal &visitor) { return visitor.visit_div(*this); }

void Div::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_div(*this);
}

mpz_class Div::constant_fold() const {
  mpz_class a = lhs->constant_fold();
  mpz_class b = rhs->constant_fold();
  if (b == 0)
    throw Error("division by 0 in " + a.get_str() + " / " + b.get_str(), loc);
  return a / b;
}

std::string Div::to_string() const {
  return "(" + lhs->to_string() + " / " + rhs->to_string() + ")";
}

Mod::Mod(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : ArithmeticBinaryExpr(lhs_, rhs_, loc_) {}

Mod *Mod::clone() const { return new Mod(*this); }

void Mod::visit(BaseTraversal &visitor) { return visitor.visit_mod(*this); }

void Mod::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_mod(*this);
}

mpz_class Mod::constant_fold() const {
  mpz_class a = lhs->constant_fold();
  mpz_class b = rhs->constant_fold();
  if (b == 0)
    throw Error("mod by 0 in " + a.get_str() + " % " + b.get_str(), loc);
  return a % b;
}

std::string Mod::to_string() const {
  return "(" + lhs->to_string() + " % " + rhs->to_string() + ")";
}

Lsh::Lsh(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : ArithmeticBinaryExpr(lhs_, rhs_, loc_) {}

Lsh *Lsh::clone() const { return new Lsh(*this); }

void Lsh::visit(BaseTraversal &visitor) { return visitor.visit_lsh(*this); }

void Lsh::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_lsh(*this);
}

// right shift an mpz value
static mpz_class rshift(mpz_class a, mpz_class b);

// left shift an mpz value
static mpz_class lshift(mpz_class a, mpz_class b) {

  // is this actually a right shift?
  if (b < 0)
    return rshift(a, -b);

  // if the shift is beyond what we can do in one shot, recurse
  while (!b.fits_ulong_p()) {
    a = lshift(a, mpz_class(ULONG_MAX));
    b -= ULONG_MAX;
  }

  // extract the shift value into a bit count
  mp_bitcnt_t l = static_cast<mp_bitcnt_t>(b.get_ui());

  // do a left shift using the GMP C API
  mpz_t rop;
  mpz_init(rop);
  mpz_mul_2exp(rop, a.get_mpz_t(), l);

  return mpz_class(rop);
}

static mpz_class rshift(mpz_class a, mpz_class b) {

  // is this actually a left shift?
  if (b < 0)
    return lshift(a, -b);

  // if the shift is beyond what we can do in one shot, recurse
  while (!b.fits_ulong_p()) {
    a = rshift(a, mpz_class(ULONG_MAX));
    b -= ULONG_MAX;
  }

  // extract the shift value into a bit count
  mp_bitcnt_t r = static_cast<mp_bitcnt_t>(b.get_ui());

  // do a right shift using the GMP C API
  mpz_t rop;
  mpz_init(rop);
  mpz_fdiv_q_2exp(rop, a.get_mpz_t(), r);

  return mpz_class(rop);
}

mpz_class Lsh::constant_fold() const {
  mpz_class a = lhs->constant_fold();
  mpz_class b = rhs->constant_fold();
  return lshift(a, b);
}

std::string Lsh::to_string() const {
  return "(" + lhs->to_string() + " << " + rhs->to_string() + ")";
}

Rsh::Rsh(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : ArithmeticBinaryExpr(lhs_, rhs_, loc_) {}

Rsh *Rsh::clone() const { return new Rsh(*this); }

void Rsh::visit(BaseTraversal &visitor) { return visitor.visit_rsh(*this); }

void Rsh::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_rsh(*this);
}

mpz_class Rsh::constant_fold() const {
  mpz_class a = lhs->constant_fold();
  mpz_class b = rhs->constant_fold();
  return rshift(a, b);
}

std::string Rsh::to_string() const {
  return "(" + lhs->to_string() + " >> " + rhs->to_string() + ")";
}

Band::Band(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : ArithmeticBinaryExpr(lhs_, rhs_, loc_) {}

Band *Band::clone() const { return new Band(*this); }

void Band::visit(BaseTraversal &visitor) { return visitor.visit_band(*this); }

void Band::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_band(*this);
}

mpz_class Band::constant_fold() const {
  mpz_class a = lhs->constant_fold();
  mpz_class b = rhs->constant_fold();
  return a & b;
}

std::string Band::to_string() const {
  return "(" + lhs->to_string() + " & " + rhs->to_string() + ")";
}

Bor::Bor(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : ArithmeticBinaryExpr(lhs_, rhs_, loc_) {}

Bor *Bor::clone() const { return new Bor(*this); }

void Bor::visit(BaseTraversal &visitor) { return visitor.visit_bor(*this); }

void Bor::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_bor(*this);
}

mpz_class Bor::constant_fold() const {
  mpz_class a = lhs->constant_fold();
  mpz_class b = rhs->constant_fold();
  return a | b;
}

std::string Bor::to_string() const {
  return "(" + lhs->to_string() + " | " + rhs->to_string() + ")";
}

Xor::Xor(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_, const location &loc_)
    : ArithmeticBinaryExpr(lhs_, rhs_, loc_) {}

Xor *Xor::clone() const { return new Xor(*this); }

void Xor::visit(BaseTraversal &visitor) { return visitor.visit_xor(*this); }

void Xor::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_xor(*this);
}

mpz_class Xor::constant_fold() const {
  mpz_class a = lhs->constant_fold();
  mpz_class b = rhs->constant_fold();
  return a ^ b;
}

std::string Xor::to_string() const {
  return "(" + lhs->to_string() + " ^ " + rhs->to_string() + ")";
}

ExprID::ExprID(const std::string &id_, const Ptr<ExprDecl> &value_,
               const location &loc_)
    : Expr(loc_), id(id_), value(value_) {}

ExprID *ExprID::clone() const { return new ExprID(*this); }

void ExprID::visit(BaseTraversal &visitor) {
  return visitor.visit_exprid(*this);
}

void ExprID::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_exprid(*this);
}

bool ExprID::constant() const {

  if (isa<ConstDecl>(value))
    return true;

  if (auto a = dynamic_cast<const AliasDecl *>(value.get()))
    return a->value->constant();

  return false;
}

Ptr<TypeExpr> ExprID::type() const {
  if (value == nullptr)
    throw Error("symbol \"" + id + "\" in expression is unresolved", loc);

  return value->get_type();
}

mpz_class ExprID::constant_fold() const {

  if (auto c = dynamic_cast<const ConstDecl *>(value.get()))
    return c->value->constant_fold();

  if (auto a = dynamic_cast<const AliasDecl *>(value.get()))
    return a->value->constant_fold();

  throw Error("symbol \"" + id + "\" is not a constant", loc);
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

bool ExprID::is_readonly() const { return value->is_readonly(); }

std::string ExprID::to_string() const { return id; }

bool ExprID::is_literal_true() const {
  // It is not possible to shadow “true,” so we simply need to check the text of
  // this expression. Boolean literals are normalised to lower case during
  // lexing (see lexer.l) so we do not need to worry about case.
  return id == "true";
}

bool ExprID::is_literal_false() const { return id == "false"; }

bool ExprID::is_pure() const { return true; }

Field::Field(const Ptr<Expr> &record_, const std::string &field_,
             const location &loc_)
    : Expr(loc_), record(record_), field(field_) {}

Field *Field::clone() const { return new Field(*this); }

void Field::visit(BaseTraversal &visitor) { return visitor.visit_field(*this); }

void Field::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_field(*this);
}

bool Field::constant() const { return false; }

Ptr<TypeExpr> Field::type() const {

  const Ptr<TypeExpr> root = record->type();
  const Ptr<TypeExpr> resolved = root->resolve();

  auto r = dynamic_cast<const Record *>(resolved.get());

  // if we are called before types have been resolved (or during type
  // resolution on a malformed model), it is possible that the root is not a
  // record
  if (r == nullptr)
    throw Error("invalid left hand side of field expression", loc);

  for (const Ptr<VarDecl> &f : r->fields) {
    if (f->name == field)
      return f->type;
  }

  throw Error("no field named \"" + field + "\" in record", loc);
}

mpz_class Field::constant_fold() const {
  throw Error("field expression used in constant", loc);
}

void Field::validate() const {

  const Ptr<TypeExpr> root = record->type()->resolve();

  if (!isa<Record>(root))
    throw Error("left hand side of field expression is not a record", loc);

  auto r = dynamic_cast<const Record &>(*root);

  for (const Ptr<VarDecl> &f : r.fields) {
    if (f->name == field)
      return;
  }

  throw Error("no field named \"" + field + "\" in record", loc);
}

bool Field::is_lvalue() const { return record->is_lvalue(); }

bool Field::is_readonly() const { return record->is_readonly(); }

std::string Field::to_string() const {
  return record->to_string() + "." + field;
}

bool Field::is_pure() const { return true; }

Element::Element(const Ptr<Expr> &array_, const Ptr<Expr> &index_,
                 const location &loc_)
    : Expr(loc_), array(array_), index(index_) {}

Element *Element::clone() const { return new Element(*this); }

void Element::visit(BaseTraversal &visitor) {
  return visitor.visit_element(*this);
}

void Element::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_element(*this);
}

bool Element::constant() const { return false; }

Ptr<TypeExpr> Element::type() const {
  const Ptr<TypeExpr> t = array->type()->resolve();
  const Array *a = dynamic_cast<const Array *>(t.get());

  // if we are called during symbol resolution on a malformed expression, our
  // left hand side may not be an array
  if (a == nullptr)
    throw Error("array reference based on something that is not an array", loc);

  return a->element_type;
}

mpz_class Element::constant_fold() const {
  throw Error("array element used in constant", loc);
}

void Element::validate() const {

  const Ptr<TypeExpr> t = array->type()->resolve();
  ;

  if (!isa<Array>(t))
    throw Error("array index on an expression that is not an array", loc);

  auto a = dynamic_cast<const Array &>(*t);

  if (!index->type()->coerces_to(*a.index_type))
    throw Error("array indexed using an expression of incorrect type", loc);
}

bool Element::is_lvalue() const { return array->is_lvalue(); }

bool Element::is_readonly() const { return array->is_readonly(); }

std::string Element::to_string() const {
  return array->to_string() + "[" + index->to_string() + "]";
}

bool Element::is_pure() const { return true; }

FunctionCall::FunctionCall(const std::string &name_,
                           const std::vector<Ptr<Expr>> &arguments_,
                           const location &loc_)
    : Expr(loc_), name(name_), arguments(arguments_) {}

FunctionCall *FunctionCall::clone() const { return new FunctionCall(*this); }

void FunctionCall::visit(BaseTraversal &visitor) {
  return visitor.visit_functioncall(*this);
}

void FunctionCall::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_functioncall(*this);
}

bool FunctionCall::constant() const {
  /* TODO: For now, we conservatively treat function calls as non-constant. In
   * future, it would be nice to lift this restriction to support more advanced
   * parameterised models.
   */
  return false;
}

Ptr<TypeExpr> FunctionCall::type() const {
  if (function == nullptr)
    throw Error("unresolved function call \"" + name + "\"", loc);

  if (function->return_type == nullptr)
    throw Error("procedure calls have no type", loc);

  return function->return_type;
}

mpz_class FunctionCall::constant_fold() const {
  // See FunctionCall::constant() regarding conservatism here.
  throw Error("function call used in a constant", loc);
}

void FunctionCall::validate() const {
  if (function == nullptr)
    throw Error("unknown function call \"" + name + "\"", loc);

  if (arguments.size() != function->parameters.size())
    throw Error("incorrect number of parameters passed to function", loc);

  if (!within_procedure_call && function->return_type == nullptr)
    throw Error("procedure (function with no return value) called in "
                "expression",
                loc);

  auto it = arguments.begin();
  for (const Ptr<VarDecl> &v : function->parameters) {

    assert(it != arguments.end() && "mismatch in size of parameter list and "
                                    "function arguments list");

    if ((*it)->is_readonly() && !v->is_readonly())
      throw Error("function call passes a read-only value as a var parameter",
                  (*it)->loc);

    if (!(*it)->type()->coerces_to(*v->get_type()))
      throw Error("function call contains parameter of incorrect type",
                  (*it)->loc);

    const Ptr<TypeExpr> param_type = v->get_type()->resolve();

    // if this is a writable range-typed parameter, we additionally require it
    // to be of exactly the same type in order to guarantee the caller’s and
    // callee’s handles are compatible
    if (!v->is_readonly() && isa<Range>(param_type)) {
      const Ptr<TypeExpr> arg_type = (*it)->type()->resolve();
      assert(isa<Range>(arg_type) &&
             "non-range considered type-compatible with range");

      auto p = dynamic_cast<const Range &>(*param_type);
      auto a = dynamic_cast<const Range &>(*arg_type);

      if (p.min->constant_fold() != a.min->constant_fold() ||
          p.max->constant_fold() != a.max->constant_fold())
        throw Error("range types of function call argument and var parameter "
                    "differ",
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

bool FunctionCall::is_pure() const {

  // If this is a recursive call within a function, it may have no referent. In
  // this case, conservatively assume it is impure.
  if (function == nullptr)
    return false;

  // if the function itself has side effects, the function call is impure
  if (!function->is_pure())
    return false;

  // if any of the parameters have side effects, the function call is impure
  for (const Ptr<Expr> &a : arguments) {
    if (!a->is_pure())
      return false;
  }

  return true;
}

Quantifier::Quantifier(const std::string &name_, const Ptr<TypeExpr> &type_,
                       const location &loc_)
    : Node(loc_), name(name_), type(type_),
      decl(Ptr<VarDecl>::make(name_, type_, loc_)) {}

Quantifier::Quantifier(const std::string &name_, const Ptr<Expr> &from_,
                       const Ptr<Expr> &to_, const location &loc_)
    : Quantifier(name_, from_, to_, nullptr, loc_) {}

Quantifier::Quantifier(const std::string &name_, const Ptr<Expr> &from_,
                       const Ptr<Expr> &to_, const Ptr<Expr> &step_,
                       const location &loc_)
    : Node(loc_), name(name_), from(from_), to(to_), step(step_),
      // we construct an artificial unbounded range here because we do not know
      // whether the bounds of this iteration are constant prior to symbol
      // resolution
      decl(Ptr<VarDecl>::make(name_, Ptr<Range>::make(nullptr, nullptr, loc_),
                              loc_)) {}

Quantifier *Quantifier::clone() const { return new Quantifier(*this); }

void Quantifier::visit(BaseTraversal &visitor) {
  return visitor.visit_quantifier(*this);
}

void Quantifier::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_quantifier(*this);
}

void Quantifier::validate() const {

  bool from_const = from != nullptr && from->constant();
  bool to_const   = to   != nullptr && to->constant();
  bool step_const = step != nullptr && step->constant();

  if (step_const && step->constant_fold() == 0)
    throw Error("infinite loop due to 0 step", loc);

  bool step_positive =
      step == nullptr || (step_const && step->constant_fold() > 0);
  bool step_negative =
      step != nullptr && step_const && step->constant_fold() < 0;

  if (from_const && to_const) {

    bool up_count   = from->constant_fold() < to->constant_fold();
    bool down_count = from->constant_fold() > to->constant_fold();

    if (up_count && step_negative)
      throw Error("infinite loop due to inverted step", loc);

    if (down_count && step_positive)
      throw Error("infinite loop due to inverted step", loc);
  }
}

std::string Quantifier::to_string() const {
  if (type == nullptr) {
    std::string s =
        name + " from " + from->to_string() + " to " + to->to_string();
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

mpz_class Quantifier::count() const {

  if (!constant())
    throw Error("non-constant quantifier is uncountable", loc);

  if (type != nullptr) {
    // subtract 1 because quantified variable can never be 'undefined'
    return type->count() - 1;
  }

  assert(from != nullptr && to != nullptr &&
         "quantifier with null type and bounds");

  mpz_class lb = from->constant_fold();
  mpz_class ub = to->constant_fold();
  mpz_class inc = step == nullptr ? 1 : step->constant_fold();

  mpz_class c = 0;
  for (mpz_class i = lb; i <= ub; i += inc)
    c++;

  return c;
}

std::string Quantifier::lower_bound() const {

  if (!constant())
    throw Error("non-constant quantifier has a lower bound that cannot be "
                "calculated ahead of time",
                loc);

  if (type != nullptr)
    return type->lower_bound();

  assert(from != nullptr && "quantifier with null type and null lower bound");

  return "VALUE_C(" + from->constant_fold().get_str() + ")";
}

bool Quantifier::is_pure() const {

  if (type != nullptr) {
    const Ptr<TypeExpr> t = type->resolve();

    if (auto r = dynamic_cast<const Range *>(t.get()))
      return r->min->is_pure() && r->max->is_pure();

    if (auto s = dynamic_cast<const Scalarset *>(t.get()))
      return s->bound->is_pure();

    assert(dynamic_cast<const Enum *>(t.get()) != nullptr &&
           "complex type encountered in quantifier");

    return true;
  }

  if (from != nullptr && !from->is_pure())
    return false;

  if (to != nullptr && !to->is_pure())
    return false;

  if (step != nullptr && !step->is_pure())
    return false;

  return true;
}

Exists::Exists(const Quantifier &quantifier_, const Ptr<Expr> &expr_,
               const location &loc_)
    : Expr(loc_), quantifier(quantifier_), expr(expr_) {}

Exists *Exists::clone() const { return new Exists(*this); }

void Exists::visit(BaseTraversal &visitor) {
  return visitor.visit_exists(*this);
}

void Exists::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_exists(*this);
}

bool Exists::constant() const { return expr->constant(); }

Ptr<TypeExpr> Exists::type() const { return Boolean; }

mpz_class Exists::constant_fold() const {
  throw Error("exists expression used in constant", loc);
}

void Exists::validate() const {
  if (!expr->is_boolean())
    throw Error("expression in exists is not boolean", expr->loc);
}

std::string Exists::to_string() const {
  return "exists " + quantifier.to_string() + " do " + expr->to_string() +
         " endexists";
}

bool Exists::is_pure() const { return quantifier.is_pure() && expr->is_pure(); }

Forall::Forall(const Quantifier &quantifier_, const Ptr<Expr> &expr_,
               const location &loc_)
    : Expr(loc_), quantifier(quantifier_), expr(expr_) {}

Forall *Forall::clone() const { return new Forall(*this); }

void Forall::visit(BaseTraversal &visitor) {
  return visitor.visit_forall(*this);
}

void Forall::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_forall(*this);
}

bool Forall::constant() const { return expr->constant(); }

Ptr<TypeExpr> Forall::type() const { return Boolean; }

mpz_class Forall::constant_fold() const {
  throw Error("forall expression used in constant", loc);
}

void Forall::validate() const {
  if (!expr->is_boolean())
    throw Error("expression in forall is not boolean", expr->loc);
}

std::string Forall::to_string() const {
  return "forall " + quantifier.to_string() + " do " + expr->to_string() +
         " endforall";
}

bool Forall::is_pure() const { return quantifier.is_pure() && expr->is_pure(); }

IsUndefined::IsUndefined(const Ptr<Expr> &expr_, const location &loc_)
    : UnaryExpr(expr_, loc_) {}

IsUndefined *IsUndefined::clone() const { return new IsUndefined(*this); }

void IsUndefined::visit(BaseTraversal &visitor) {
  return visitor.visit_isundefined(*this);
}

void IsUndefined::visit(ConstBaseTraversal &visitor) const {
  return visitor.visit_isundefined(*this);
}

bool IsUndefined::constant() const { return false; }

Ptr<TypeExpr> IsUndefined::type() const { return Boolean; }

mpz_class IsUndefined::constant_fold() const {
  throw Error("isundefined used in constant", loc);
}

void IsUndefined::validate() const {

  if (!rhs->is_lvalue())
    throw Error("non-lvalue expression cannot be used in isundefined",
                rhs->loc);

  const Ptr<TypeExpr> t = rhs->type();
  if (!t->is_simple())
    throw Error("complex type used in isundefined", rhs->loc);
}

std::string IsUndefined::to_string() const {
  return "isundefined(" + rhs->to_string() + ")";
}

} // namespace rumur
