#include <cstddef>
#include "location.hh"
#include <memory>
#include <rumur/Boolean.h>
#include <rumur/Expr.h>
#include <rumur/Ptr.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>

namespace rumur {

const Ptr<Enum> Boolean = Ptr<Enum>::make(
  std::vector<std::pair<std::string, location>>(
    { {"false", location()}, {"true", location()} }), location());

}
