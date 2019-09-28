#include <cassert>
#include <cstddef>
#include <iostream>
#include "location.hh"
#include "parser.yy.hh"
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Model.h>
#include <rumur/parse.h>
#include <rumur/Ptr.h>
#include <rumur/scanner.h>

namespace rumur {

Ptr<Model> parse(std::istream *input) {

  assert(input != nullptr);

  // Setup the parser
  scanner s(input);
  Ptr<Model> m;
  parser p(s, m);

  // Parse the input model
  int err = p.parse();
  if (err != 0)
    throw Error("parsing failed", location());

  // mark the global declarations
  for (Ptr<Decl> &d : m->decls) {
    if (auto v = dynamic_cast<VarDecl*>(&*d)) {
      v->state_variable = true;
    }
  }

  return m;
}

}
