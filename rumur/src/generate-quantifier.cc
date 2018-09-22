#include <cstddef>
#include "generate.h"
#include <gmpxx.h>
#include <iostream>
#include <rumur/rumur.h>
#include <string>

using namespace rumur;

void generate_quantifier_header(std::ostream &out, const Quantifier &q) {

  std::string const counter = "_ru1_" + q.var->name;
  std::string const lb = q.var->type->lower_bound();
  std::string const ub = q.var->type->upper_bound();
  std::string const inc = q.step == nullptr
    ? "VALUE_C(1)"
    : "VALUE_C(" + q.step->constant_fold().get_str() + ")";

  std::string const block = "_ru2_" + q.var->name;
  mpz_class width = q.var->type->width();

  std::string const handle = "ru_" + q.var->name;

  /* Set up quantifiers. It might be surprising to notice that there is an extra
   * level of indirection here. A variable 'x' results in loop counter '_ru1_x',
   * storage array '_ru2_x' and handle 'ru_x'. We use three variables rather
   * than two in order to avoid rules that modify the ruleset parameters
   * (uncommon) affecting the loop counter.
   */
  out
    << "for (value_t " << counter << " = " << lb << "; " << counter << " <= "
      << ub << "; " << counter << " += " << inc << ") {\n"
    << "  uint8_t " << block << "[BITS_TO_BYTES(" << width << ")] = { 0 };\n"
    << "  struct handle " << handle << " = { .base = " << block
      << ", .offset = 0, .width = SIZE_C(" << width << ") };\n"
    << "  handle_write(s, " << lb << ", " << ub << ", " << handle << ", "
      << counter << ");\n";
}

void generate_quantifier_footer(std::ostream &out, const Quantifier &q) {

  std::string const counter = "_ru1_" + q.var->name;
  std::string const ub = q.var->type->upper_bound();
  std::string const inc = q.step == nullptr
    ? "VALUE_C(1)"
    : "VALUE_C(" + q.step->constant_fold().get_str() + ")";

  out
    << "  if (VALUE_MAX - " << inc << " < " << ub << " && " << counter
      << " > VALUE_MAX - " << inc << ") {\n"
    << "    break;\n"
    << "  }\n"
    << "}\n";
}
