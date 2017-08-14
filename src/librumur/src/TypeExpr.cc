#include <memory>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>

using namespace rumur;
using namespace std;

Range::Range(shared_ptr<Expr> min, shared_ptr<Expr> max, const location &loc)
  : TypeExpr(loc), min(min), max(max) {
}

TypeExprID::TypeExprID(const string &id, shared_ptr<TypeExpr> value,
  const location &loc)
  : TypeExpr(loc), id(id), value(value) {
}

Enum::Enum(const vector<pair<string, location>> &members, const location &loc)
  : TypeExpr(loc) {

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
