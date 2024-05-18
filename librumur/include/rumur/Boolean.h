#pragma once

#include <cstddef>
#include <memory>
#include <rumur/Expr.h>
#include <rumur/Ptr.h>
#include <rumur/TypeExpr.h>

#ifndef RUMUR_API
#define RUMUR_API __attribute__((visibility("default")))
#endif

namespace rumur {

/// the built in boolean type that is implicitly declared in all Murphi models
extern RUMUR_API const Ptr<Enum> Boolean;

/// literals for Murphi “False” and “True”
///
/// These are included for convenience, so you can assign to expressions to
/// these constants if relevant. Do not compare against these expressions. If
/// you want to ask “is this expression the literal true/false?” use
/// `Expr::is_literal_true` / `Expr::is_literal_false`.
extern RUMUR_API const Ptr<Expr> False;
extern RUMUR_API const Ptr<Expr> True;

} // namespace rumur
