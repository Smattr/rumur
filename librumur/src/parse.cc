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
  Ptr<Node> answer;
  parser p(s, answer, parser::token::START_MODEL);

  // Parse the input model
  int err = p.parse();
  if (err != 0)
    throw Error("parsing failed", location());

  // transform this into a Model pointer
  Ptr<Model> model = answer.narrow<Model>();
  assert(model != NULL);

  return model;
}
