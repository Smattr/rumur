#pragma once

#include <cstddef>
#include <iostream>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Model.h>
#include <rumur/Property.h>
#include <rumur/Ptr.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>

#ifndef RUMUR_API
#define RUMUR_API __attribute__((visibility("default")))
#endif

namespace rumur {

// Parse in a model from an input stream. Throws Errors on parsing errors.
RUMUR_API Ptr<Model> parse_model(std::istream &input);

// parse other partial fragments of Murphi
RUMUR_API Ptr<Decl> parse_decl(std::istream &input);
RUMUR_API Ptr<Expr> parse_expr(std::istream &input);
RUMUR_API Ptr<Property> parse_property(std::istream &input);
RUMUR_API Ptr<Rule> parse_rule(std::istream &input);
RUMUR_API Ptr<Stmt> parse_stmt(std::istream &input);

} // namespace rumur
