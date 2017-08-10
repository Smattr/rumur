#include "location.hh"
#include <rumur/Boolean.h>
#include <rumur/Expr.h>
#include <rumur/TypeExpr.h>
#include <vector>

using namespace std;

namespace rumur {

static ExprID *False = new ExprID("false", nullptr, nullptr, location());
static ExprID *True = new ExprID("true", nullptr, nullptr, location());

const Enum Boolean(vector<ExprID*>{False, True}, location());

}
