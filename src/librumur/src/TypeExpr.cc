#include <memory>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>

using namespace rumur;
using namespace std;

bool TypeExpr::is_simple() const {
    return false;
}

bool SimpleTypeExpr::is_simple() const {
    return true;
}

Range::Range(shared_ptr<Expr> min, shared_ptr<Expr> max, const location &loc)
  : SimpleTypeExpr(loc), min(min), max(max) {
}

void Range::validate() const {
    if (!min->constant())
        throw RumurError("lower bound of range is not a constant", min->loc);

    if (!max->constant())
        throw RumurError("upper bound of range is not a constant", max->loc);
}

TypeExprID::TypeExprID(const string &id, shared_ptr<TypeExpr> value,
  const location &loc)
  : TypeExpr(loc), id(id), value(value) {
}

bool TypeExprID::is_simple() const {
    return value->is_simple();
}

Enum::Enum(const vector<pair<string, location>> &members, const location &loc)
  : SimpleTypeExpr(loc) {

    for (auto [s, l] : members) {

        // Assign the enum member a numerical value
        auto n = make_shared<Number>(this->members.size(), l);

        // Construct an expression for it
        auto e = make_shared<ExprID>(s, n, this, l);
        this->members.emplace_back(e);

    }
}

Record::Record(vector<shared_ptr<VarDecl>> &&fields, const location &loc)
  : TypeExpr(loc), fields(fields) {
}
