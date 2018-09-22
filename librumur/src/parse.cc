#include <cassert>
#include <cstddef>
#include <iostream>
#include "location.hh"
#include <memory>
#include "parser.yy.hh"
#include <rumur/except.h>
#include <rumur/Model.h>
#include <rumur/parse.h>
#include <rumur/scanner.h>

namespace rumur {

std::shared_ptr<Model> parse(std::istream *input) {

  assert(input != nullptr);

  // Setup the parser
  scanner s(input);
  std::shared_ptr<rumur::Model> m;
  parser p(s, m);

  // Parse the input model
  int err = p.parse();
  if (err != 0)
    throw Error("parsing failed", location());

  return m;
}

}
