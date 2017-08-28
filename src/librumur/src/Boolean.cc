#include "location.hh"
#include <rumur/Boolean.h>
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace rumur {

// XXX: We assume the boolean type doesn't need any valid indexing
static Indexer indexer;

static const pair<string, location> False = make_pair("false", location());
static const pair<string, location> True = make_pair("true", location());

static const vector<pair<string, location>> members{False, True};

const Enum Boolean(members, location(), indexer);

}
