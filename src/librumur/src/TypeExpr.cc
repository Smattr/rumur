#include <rumur/TypeExpr.h>

using namespace rumur;

Range::Range(Expr *min, Expr *max, const location &loc)
  : TypeExpr(loc), min(min), max(max) {
}

Range::~Range() {
    delete min;
    delete max;
}
