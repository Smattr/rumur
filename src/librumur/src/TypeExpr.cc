#include <rumur/Expr.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>

using namespace rumur;
using namespace std;

Range::Range(Expr *min, Expr *max, const location &loc)
  : TypeExpr(loc), min(min), max(max) {
}

Range::~Range() {
    delete min;
    delete max;
}

TypeExprID::TypeExprID(const string &id, const TypeExpr *value, const location &loc)
  : TypeExpr(loc), id(id), value(value) {
}

Enum::Enum(const vector<pair<string, location>> &members, const location &loc)
  : TypeExpr(loc) {

    for (auto [s, l] : members) {

        // Assign the enum member a numerical value
        Number *n = new Number(this->members.size(), l);

        // Construct an expression for it
        ExprID *e = new ExprID(s, n, this, l);
        this->members.emplace_back(e);

    }
}

Enum::~Enum() {
    for (ExprID *e : members) {
        /* Note, our ExprIDs are special in that we own their value and so have
         * responsibility for deleting it.
         */
        delete e->value;
        delete e;
    }
}
