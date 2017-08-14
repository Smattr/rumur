#include <cstdint>
#include "location.hh"
#include <memory>
#include <rumur/Boolean.h>
#include <rumur/Expr.h>
#include <rumur/TypeExpr.h>
#include <string>

using namespace rumur;
using namespace std;

Expr::~Expr() {
}

Ternary::Ternary(shared_ptr<Expr> cond, shared_ptr<Expr> lhs,
  shared_ptr<Expr> rhs, const location &loc) noexcept
  : Expr(loc), cond(cond), lhs(lhs), rhs(rhs) {
}

bool Ternary::constant() const noexcept {
    return cond->constant() && lhs->constant() && rhs->constant();
}

const TypeExpr *Ternary::type() const noexcept {
    // TODO: assert lhs and rhs are compatible types.
    return lhs->type();
}

BinaryExpr::BinaryExpr(shared_ptr<Expr> lhs, shared_ptr<Expr> rhs,
  const location &loc) noexcept
  : Expr(loc), lhs(lhs), rhs(rhs) {
}

bool BinaryExpr::constant() const noexcept {
    return lhs->constant() && rhs->constant();
}

const TypeExpr *Implication::type() const noexcept {
    return &Boolean;
}

const TypeExpr *Or::type() const noexcept {
    return &Boolean;
}

const TypeExpr *And::type() const noexcept {
    return &Boolean;
}

UnaryExpr::UnaryExpr(shared_ptr<Expr> rhs, const location &loc) noexcept
  : Expr(loc), rhs(rhs) {
}

bool UnaryExpr::constant() const noexcept {
    return rhs->constant();
}

const TypeExpr *Not::type() const noexcept {
    return &Boolean;
}

const TypeExpr *Lt::type() const noexcept {
    return &Boolean;
}

const TypeExpr *Leq::type() const noexcept {
    return &Boolean;
}

const TypeExpr *Gt::type() const noexcept {
    return &Boolean;
}

const TypeExpr *Geq::type() const noexcept {
    return &Boolean;
}

const TypeExpr *Eq::type() const noexcept {
    return &Boolean;
}

const TypeExpr *Neq::type() const noexcept {
    return &Boolean;
}

const TypeExpr *Add::type() const noexcept {
    return nullptr;
}

const TypeExpr *Sub::type() const noexcept {
    return nullptr;
}

const TypeExpr *Negative::type() const noexcept {
    return rhs->type();
}

const TypeExpr *Mul::type() const noexcept {
    return nullptr;
}

const TypeExpr *Div::type() const noexcept {
    return nullptr;
}

const TypeExpr *Mod::type() const noexcept {
    return nullptr;
}

ExprID::ExprID(const string &id, shared_ptr<Expr> value,
  const TypeExpr *type_of, const location &loc)
  : Expr(loc), id(id), value(value), type_of(type_of) {
}

bool ExprID::constant() const noexcept {
    return value->constant();
}

const TypeExpr *ExprID::type() const noexcept {
    return type_of;
}

Field::Field(shared_ptr<Expr> record, const string &field, const location &loc)
  : Expr(loc), record(record), field(field) {
}

bool Field::constant() const noexcept {
    return record->constant();
}

const TypeExpr *Field::type() const noexcept {
    // TODO
    return nullptr;
}

Element::Element(shared_ptr<Expr> array, shared_ptr<Expr> index, const location &loc)
  : Expr(loc), array(array), index(index) {
}

bool Element::constant() const noexcept {
    return array->constant() && index->constant();
}

const TypeExpr *Element::type() const noexcept {
    // TODO
    return nullptr;
}
