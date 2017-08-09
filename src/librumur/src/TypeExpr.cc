#include <rumur/Expr.h>
#include <rumur/TypeExpr.h>
#include <string>
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

TypeExprID::TypeExprID(const string &id, TypeExpr *value, const location &loc)
  : TypeExpr(loc), id(id), value(value) {
}

TypeExprID::~TypeExprID() {
    delete value;
}

Enum::Enum(vector<EnumValue*> &&members, const location &loc)
  : TypeExpr(loc), members(members) {
}

Enum::~Enum() {
    for (EnumValue *e : members)
        delete e;
}
