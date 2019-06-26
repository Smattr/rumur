#include <cstddef>
#include "location.hh"
#include <memory>
#include <rumur/Boolean.h>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Number.h>
#include <rumur/Ptr.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>

namespace rumur {

const Ptr<Enum> Boolean = Ptr<Enum>::make(
  std::vector<std::pair<std::string, location>>(
    { {"false", location()}, {"true", location()} }), location());

const Ptr<Expr> False = Ptr<ExprID>::make("false",
  Ptr<ConstDecl>::make("boolean",
    Ptr<Number>::make(0, location()),
  Boolean, location()),
location());

const Ptr<Expr> True = Ptr<ExprID>::make("true",
  Ptr<ConstDecl>::make("boolean",
    Ptr<Number>::make(1, location()),
  Boolean, location()),
location());

}
