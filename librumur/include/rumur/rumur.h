// A header for lazy folks who just want all of librumur's functionality.

#pragma once

#include "location.hh"
#include "parser.yy.hh"
#include "position.hh"
#include <rumur/Boolean.h>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Function.h>
#include "rumur-get-version.h" // generated
#include <rumur/indexer.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <rumur/parse.h>
#include <rumur/Property.h>
#include <rumur/Ptr.h>
#include <rumur/resolve-symbols.h>
#include <rumur/Rule.h>
#include <rumur/scanner.h>
#include <rumur/smt.h>
#include <rumur/Stmt.h>
#include <rumur/Symtab.h>
#include <rumur/traverse.h>
#include <rumur/TypeExpr.h>
#include <rumur/validate.h>
// stack.hh is deliberately not included; just use std::stack
