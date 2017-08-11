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

TypeExprID::TypeExprID(const string &id, const TypeExpr *value, const location &loc)
  : TypeExpr(loc), id(id), value(value) {
}

Enum::Enum(vector<ExprID*> &&members, const location &loc)
  : TypeExpr(loc), members(members) {
    int64_t i = 0;
    for (ExprID *e : members) {

        // Assign the enum member a numerical value
        Number *n = new Number(i, e->loc);
        e->value = n;

        // Give it the correct type
        e->type_of = this;

        i++;
    }
    /* Now we have repaired the invariants that Symtab (or one of its callers)
     * expects.
     */
}

Enum::~Enum() {
    for (Number *n : representations)
        delete n;
    for (ExprID *e : members)
        delete e;
}
