#include "location.hh"
#include <rumur/Boolean.h>
#include <rumur/Expr.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace rumur {

static const pair<string, location> False = make_pair("false", location());
static const pair<string, location> True = make_pair("true", location());

static const vector<pair<string, location>> members{False, True};

const Enum Boolean(members, location());

}
