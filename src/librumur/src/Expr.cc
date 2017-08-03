#include "location.hh"
#include <rumur/Expr.h>

using namespace rumur;

Expr::~Expr() {
}

Ternary::Ternary(Expr *cond, Expr *lhs, Expr *rhs, const location &loc) noexcept
  : Expr(loc), cond(cond), lhs(lhs), rhs(rhs) {
}

bool Ternary::constant() const noexcept {
    return cond->constant() && lhs->constant() && rhs->constant();
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

UnaryExpr::UnaryExpr(Expr *rhs, const location &loc) noexcept
  : Expr(loc), rhs(rhs) {
}

bool UnaryExpr::constant() const noexcept {
    return rhs->constant();
}

UnaryExpr::~UnaryExpr() {
    delete rhs;
}
