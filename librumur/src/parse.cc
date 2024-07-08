#include "location.hh"
#include "parser.yy.hh"
#include <cassert>
#include <cstddef>
#include <iostream>
#include <rumur/Decl.h>
#include <rumur/Model.h>
#include <rumur/Ptr.h>
#include <rumur/except.h>
#include <rumur/parse.h>
#include <rumur/scanner.h>

using namespace rumur;

Ptr<Model> rumur::parse(std::istream &input) {

  // Setup the parser
  scanner s(&input);
  Ptr<Model> m;
  parser p(s, m, parser::token::START_MODEL);

  // Parse the input model
  int err = p.parse();
  if (err != 0)
    throw Error("parsing failed", location());

  return m;
}
