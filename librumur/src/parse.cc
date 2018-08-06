#include <cassert>
#include <iostream>
#include "location.hh"
#include "parser.yy.hh"
#include <rumur/Boolean.h>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <rumur/parse.h>
#include <rumur/scanner.h>
#include <rumur/Symtab.h>
#include <rumur/TypeExpr.h>

namespace rumur {

Model *parse(std::istream *input) {

  assert(input != nullptr);

  // Setup a symbol table that knows the built ins
  Symtab symtab;
  symtab.open_scope();
  symtab.declare("boolean", Boolean);
  for (const std::pair<std::string, location> &m : Boolean.members)
    symtab.declare(m.first, TypeDecl("boolean", new Enum(Boolean), location()));

  // Setup the parser
  scanner s(input);
  Model *m = nullptr;
  parser p(s, m, symtab);

  // Parse the input model
  int err = p.parse();
  if (err != 0)
    throw Error("parsing failed", location());

  return m;
}

}