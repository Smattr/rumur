#include <cassert>
#include <iostream>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>
#include <utility>

namespace rumur {

Assignment::Assignment(Lvalue *lhs, Expr *rhs, const location &loc, Indexer&):
    Stmt(loc), lhs(lhs), rhs(rhs) {
}

Assignment::Assignment(const Assignment &other):
    Stmt(other), lhs(other.lhs->clone()), rhs(other.rhs->clone()) {
}

Assignment &Assignment::operator=(Assignment other) {
    swap(*this, other);
    return *this;
}

void swap(Assignment &x, Assignment &y) noexcept {
    using std::swap;
    swap(x.loc, y.loc);
    swap(x.lhs, y.lhs);
    swap(x.rhs, y.rhs);
}

Assignment *Assignment::clone() const {
    return new Assignment(*this);
}

Assignment::~Assignment() {
    delete lhs;
    delete rhs;
}

void Assignment::validate() const {
    lhs->validate();
    if (dynamic_cast<const SimpleTypeExpr*>(lhs->type()) == nullptr)
        throw RumurError("left hand side of assignment does not have a simple "
          "type", lhs->loc);
    rhs->validate();
}

void Assignment::generate_stmt(std::ostream &out) const {
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

}
