#include "location.hh"
#include <memory>
#include <rumur/Boolean.h>
#include <rumur/Expr.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>

namespace rumur {

const std::shared_ptr<Enum> Boolean = std::make_shared<Enum>(
  std::vector<std::pair<std::string, location>>(
    { {"false", location()}, {"true", location()} }), location());

}
