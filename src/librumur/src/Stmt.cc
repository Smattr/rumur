#include <memory>
#include <rumur/Expr.h>
#include <rumur/Stmt.h>

using namespace rumur;
using namespace std;

Assignment::Assignment(shared_ptr<Expr> lhs, shared_ptr<Expr> rhs,
  const location &loc)
  : Stmt(loc), lhs(lhs), rhs(rhs) {
}
