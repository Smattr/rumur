#include "location.hh"
#include <rumur/Expr.h>

using namespace rumur;

Expr::~Expr() {
}

Ternary::Ternary(Expr *cond, Expr *lhs, Expr *rhs, const location &loc) noexcept
  : Expr(loc), cond(cond), lhs(lhs), rhs(rhs) {
}

Ternary::~Ternary() {
    delete cond;
    delete lhs;
    delete rhs;
}

BinaryExpr::BinaryExpr(Expr *lhs, Expr *rhs, const location &loc) noexcept
  : Expr(loc), lhs(lhs), rhs(rhs) {
}

BinaryExpr::~BinaryExpr() {
    delete lhs;
    delete rhs;
}

UnaryExpr::UnaryExpr(Expr *rhs, const location &loc) noexcept
  : Expr(loc), rhs(rhs) {
}

UnaryExpr::~UnaryExpr() {
    delete rhs;
}
