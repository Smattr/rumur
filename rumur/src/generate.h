#pragma once

#include <cstddef>
#include <gmpxx.h>
#include <memory>
#include <rumur/rumur.h>
#include <string>
#include <vector>

int output_checker(const std::string &path, const rumur::Model &model);

// Generate prelude definitions to allocate memory for function returns
void generate_allocations(std::ostream &out, const rumur::Stmt &stmt);

// Helper for calling the above on a body of functions
void generate_allocations(std::ostream &out,
  const std::vector<rumur::Ptr<rumur::Stmt>> &stmts);

// Generate definition of a ConstDecl or VarDecl
void generate_decl(std::ostream &out, const rumur::Decl &d);

void generate_function(std::ostream &out, const rumur::Function &f,
  const std::vector<std::shared_ptr<rumur::Decl>> &decls);

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
