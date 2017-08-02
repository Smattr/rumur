#include <rumur/Expr.h>

using namespace rumur;

Expr::~Expr() {
}

Ternary::Ternary(Expr *cond, Expr *lhs, Expr *rhs) noexcept
  : cond(cond), lhs(lhs), rhs(rhs) {
}

Ternary::~Ternary() {
    delete cond;
    delete lhs;
    delete rhs;
}

BinaryExpr::BinaryExpr(Expr *lhs, Expr *rhs) noexcept
  : lhs(lhs), rhs(rhs) {
}

BinaryExpr::~BinaryExpr() {
    delete lhs;
    delete rhs;
}

UnaryExpr::UnaryExpr(Expr *rhs) noexcept
  : rhs(rhs) {
}

UnaryExpr::~UnaryExpr() {
    delete rhs;
}
