#include "location.hh"
#include <cstddef>
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

using namespace rumur;

const Ptr<Enum> rumur::Boolean =
    Ptr<Enum>::make(std::vector<std::pair<std::string, location>>(
                        {{"false", location()}, {"true", location()}}),
                    location());

const Ptr<Expr> rumur::False = Ptr<ExprID>::make(
    "false",
    Ptr<ConstDecl>::make("boolean", Ptr<Number>::make(0, location()), Boolean,
                         location()),
    location());

const Ptr<Expr> rumur::True = Ptr<ExprID>::make(
    "true",
    Ptr<ConstDecl>::make("boolean", Ptr<Number>::make(1, location()), Boolean,
                         location()),
    location());
