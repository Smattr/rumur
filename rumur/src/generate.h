#pragma once

#include <rumur/rumur.h>
#include <string>

int output_checker(const std::string &path, const rumur::Model &model);

void generate_model(std::ostream &out, const rumur::Model &m);

void generate_stmt(std::ostream &out, const rumur::Stmt &s);
