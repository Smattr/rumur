#pragma once

#include <cstddef>
#include <gmpxx.h>
#include <string>

namespace smt {

std::string integer_type();

std::string numeric_literal(const mpz_class &value);

std::string add();
std::string div();
std::string geq();
std::string gt ();
std::string leq();
std::string lsh();
std::string lt ();
std::string mod();
std::string mul();
std::string neg();
std::string rsh();
std::string sub();

}
