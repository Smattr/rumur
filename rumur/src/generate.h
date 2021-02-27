#pragma once

#include "ValueType.h"
#include <cstddef>
#include <gmpxx.h>
#include <memory>
#include <rumur/rumur.h>
#include <string>
#include <utility>
#include <vector>

int output_checker(const std::string &path, const rumur::Model &model,
                   const std::pair<ValueType, ValueType> &value_types);

// Generate prelude definitions to allocate memory for function returns
void generate_allocations(std::ostream &out, const rumur::Stmt &stmt);

// Helper for calling the above on a body of functions
void generate_allocations(std::ostream &out,
                          const std::vector<rumur::Ptr<rumur::Stmt>> &stmts);

// Generate definition of a ConstDecl or VarDecl
void generate_decl(std::ostream &out, const rumur::Decl &d);

void generate_function(std::ostream &out, const rumur::Function &f,
                       const std::vector<const rumur::Decl *> &decls);

void generate_model(std::ostream &out, const rumur::Model &m);

// Generate C code to print the value of the given type at the given handle.
void generate_print(std::ostream &out, const rumur::TypeExpr &e,
                    const std::string &prefix, const std::string &handle,
                    bool support_diff, bool support_xml);

void generate_property(std::ostream &out, const rumur::Property &p);

void generate_lvalue(std::ostream &out, const rumur::Expr &e);
void generate_rvalue(std::ostream &out, const rumur::Expr &e);

void generate_quantifier_header(std::ostream &out, const rumur::Quantifier &q);
void generate_quantifier_footer(std::ostream &out, const rumur::Quantifier &q);

void generate_stmt(std::ostream &out, const rumur::Stmt &s);

void generate_cover_array(std::ostream &out, const rumur::Model &model);
