#include <iostream>
#include <memory>
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/Stmt.h>
#include <string>

using namespace rumur;
using namespace std;

Assignment::Assignment(shared_ptr<Expr> lhs, shared_ptr<Expr> rhs,
  const location &loc, Indexer&)
  : Stmt(loc), lhs(lhs), rhs(rhs) {
}

void Assignment::validate() const {
    lhs->validate();
    rhs->validate();
}

void Assignment::generate_stmt(ostream &out, const string &indent) const {
    out << indent << "/* TODO */ = /* TODO */;";
}
