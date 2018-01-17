#include "location.hh"
#include <rumur/Boolean.h>
#include <rumur/Expr.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>

namespace rumur {

static const std::pair<std::string, location> False = std::make_pair("false", location());
static const std::pair<std::string, location> True = std::make_pair("true", location());

static const std::vector<std::pair<std::string, location>> members{False, True};

const Enum Boolean(members, location());

}
