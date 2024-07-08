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

template <typename T, int STARTER>
static Ptr<T> parse_node(std::istream &input) {

  // Setup the parser
  scanner s(&input);
  Ptr<Node> answer;
  parser p(s, answer, STARTER);

  // Parse the input model
  int err = p.parse();
  if (err != 0)
    throw Error("parsing failed", location());

  // transform this into a narrowed pointer
  Ptr<T> specific = answer.narrow<T>();
  assert(specific != NULL);

  return specific;
}

Ptr<Decl> rumur::parse_decl(std::istream &input) {
  return parse_node<Decl, parser::token::START_DECL>(input);
}

Ptr<Expr> rumur::parse_expr(std::istream &input) {
  return parse_node<Expr, parser::token::START_EXPR>(input);
}

Ptr<Model> rumur::parse_model(std::istream &input) {
  return parse_node<Model, parser::token::START_MODEL>(input);
}

Ptr<Property> rumur::parse_property(std::istream &input) {
  return parse_node<Property, parser::token::START_PROPERTY>(input);
}

Ptr<Rule> rumur::parse_rule(std::istream &input) {
  return parse_node<Rule, parser::token::START_RULE>(input);
}

Ptr<Stmt> rumur::parse_stmt(std::istream &input) {
  return parse_node<Stmt, parser::token::START_STMT>(input);
}
