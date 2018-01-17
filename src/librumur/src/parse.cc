#include <cassert>
#include <iostream>
#include "location.hh"
#include "parser.yy.hh"
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
  const Enum boolean({ { "false", location() }, { "true", location() } },
    location());
  symtab.declare("boolean", boolean);
  for (const ExprID &eid : boolean.members)
    symtab.declare(eid.id, eid);

  // Setup the parser
  scanner s(input);
  Model *m = nullptr;
  parser p(s, m, symtab);

  // Parse the input model
  int err = p.parse();
  if (err != 0)
    throw RumurError("parsing failed", location());

  /* Validate that the model makes sense. E.g. constant definitions are
   * actually constant.
   */
  assert(m != nullptr);
  m->validate();

  return m;
}

}
