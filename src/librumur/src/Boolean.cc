#include "location.hh"
#include <rumur/Boolean.h>
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>

namespace rumur {

// XXX: We assume the boolean type doesn't need any valid indexing
static Indexer indexer;

static const std::pair<std::string, location> False = std::make_pair("false", location());
static const std::pair<std::string, location> True = std::make_pair("true", location());

static const std::vector<std::pair<std::string, location>> members{False, True};

const Enum Boolean(members, location(), indexer);

}
