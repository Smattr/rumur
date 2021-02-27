#pragma once

#include <cstddef>
#include <memory>
#include <rumur/Expr.h>
#include <rumur/Ptr.h>
#include <rumur/TypeExpr.h>

namespace rumur {

/// the built in boolean type that is implicitly declared in all Murphi models
extern const Ptr<Enum> Boolean;

/// literals for Murphi “False” and “True”
///
/// These are included for convenience, so you can assign to expressions to
/// these constants if relevant.
extern const Ptr<Expr> False;
extern const Ptr<Expr> True;

} // namespace rumur
