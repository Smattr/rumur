#include <rumur/Expr.h>
#include <rumur/Stmt.h>

using namespace rumur;

Assignment::Assignment(Expr *lhs, Expr *rhs, const location &loc)
  : Stmt(loc), lhs(lhs), rhs(rhs) {
}

Assignment::~Assignment() {
    delete lhs;
    delete rhs;
}
