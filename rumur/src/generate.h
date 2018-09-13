#pragma once

#include <gmpxx.h>
#include <rumur/rumur.h>
#include <string>

int output_checker(const std::string &path, const rumur::Model &model);

void generate_function(std::ostream &out, const rumur::Function &f);

void generate_model(std::ostream &out, const rumur::Model &m);

// Generate C code to print the value of the given state variable
void generate_print(std::ostream &out, const rumur::VarDecl &d,
  const std::string &prefix, mpz_class preceding_offset);

void generate_property(std::ostream &out, const rumur::Property &p);

void generate_lvalue(std::ostream &out, const rumur::Expr &e);
void generate_rvalue(std::ostream &out, const rumur::Expr &e);

void generate_quantifier_header(std::ostream &out, const rumur::Quantifier &q);
void generate_quantifier_footer(std::ostream &out, const rumur::Quantifier &q);

void generate_stmt(std::ostream &out, const rumur::Stmt &s);
