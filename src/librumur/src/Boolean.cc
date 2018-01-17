#include "location.hh"
#include <rumur/Boolean.h>
#include <rumur/Expr.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>

namespace rumur {

const Enum Boolean({ {"false", location()}, {"true", location()} }, location());

}
