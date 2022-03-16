// Rumur exported API

#pragma once

#ifndef RUMUR_API
#define RUMUR_API
#endif

namespace rumur {

// forward declaration to update visibility
class RUMUR_API parser;

} // namespace rumur

#include "location.hh"
#include "parser.yy.hh"
#include "position.hh"
#include "rumur-get-version.h" // generated
#include <cstddef>
#include <rumur/Boolean.h>
#include <rumur/Comment.h>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Function.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <rumur/Property.h>
#include <rumur/Ptr.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/Symtab.h>
#include <rumur/TypeExpr.h>
#include <rumur/except.h>
#include <rumur/indexer.h>
#include <rumur/parse.h>
#include <rumur/resolve-symbols.h>
#include <rumur/sanitise_rule_names.h>
#include <rumur/scanner.h>
#include <rumur/traverse.h>
#include <rumur/validate.h>
// stack.hh is deliberately not included; just use std::stack
