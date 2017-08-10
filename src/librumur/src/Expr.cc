#include <cstdint>
#include "location.hh"
#include <rumur/Boolean.h>
#include <rumur/Expr.h>
#include <rumur/TypeExpr.h>
#include <string>

using namespace rumur;
using namespace std;

Expr::~Expr() {
}

Ternary::Ternary(Expr *cond, Expr *lhs, Expr *rhs, const location &loc) noexcept
  : Expr(loc), cond(cond), lhs(lhs), rhs(rhs) {
}

bool Ternary::constant() const noexcept {
    return cond->constant() && lhs->constant() && rhs->constant();
}

const TypeExpr *Ternary::type() const noexcept {
    // TODO: assert lhs and rhs are compatible types.
    return lhs->type();
}

Ternary::~Ternary() {
    delete cond;
    delete lhs;
    delete rhs;
}

BinaryExpr::BinaryExpr(Expr *lhs, Expr *rhs, const location &loc) noexcept
  : Expr(loc), lhs(lhs), rhs(rhs) {
}

bool BinaryExpr::constant() const noexcept {
    return lhs->constant() && rhs->constant();
}

BinaryExpr::~BinaryExpr() {
    delete lhs;
    delete rhs;
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

UnaryExpr::UnaryExpr(Expr *rhs, const location &loc) noexcept
  : Expr(loc), rhs(rhs) {
}

bool UnaryExpr::constant() const noexcept {
    return rhs->constant();
}

UnaryExpr::~UnaryExpr() {
    delete rhs;
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

ExprID::ExprID(const string &id, Expr *value, const TypeExpr *type_of, const location &loc)
  : Expr(loc), id(id), value(value), type_of(type_of) {
}

bool ExprID::constant() const noexcept {
    return value->constant();
}

const TypeExpr *ExprID::type() const noexcept {
    return type_of;
}
