#include <cassert>
#include <iostream>
#include <memory>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>
#include <string>

using namespace rumur;
using namespace std;

Assignment::Assignment(shared_ptr<Lvalue> lhs, shared_ptr<Expr> rhs,
  const location &loc, Indexer&)
  : Stmt(loc), lhs(lhs), rhs(rhs) {
}

void Assignment::validate() const {
    lhs->validate();
    if (dynamic_cast<const SimpleTypeExpr*>(lhs->type()) == nullptr)
        throw RumurError("left hand side of assignment does not have a simple "
          "type", lhs->loc);
    rhs->validate();
}

void Assignment::generate_stmt(ostream &out) const {
    auto t = dynamic_cast<const SimpleTypeExpr*>(lhs->type());
    assert (t != nullptr && "BUG: Left hand side of assignment with non-simple "
      "type. Attempted code generation on unvalidated AST?");
    t->writer(out);
    out << "(";
    lhs->lvalue(out);
    out << ",";
    rhs->rvalue(out);
    out << ");";
}
