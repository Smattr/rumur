#include <cassert>
#include <cstddef>
#include "generate.h"
#include <iostream>
#include <rumur/rumur.h>
#include <string>
#include "utils.h"

using namespace rumur;

void generate_quantifier_header(std::ostream &out, const Quantifier &q) {

  /* Set up quantifiers. It might be surprising to notice that there is an extra
   * level of indirection here. A variable 'x' results in loop counter '_ru1_x',
   * storage array '_ru2_x' and handle 'ru_x'. We use three variables rather
   * than two in order to avoid rules that modify the ruleset parameters
   * (uncommon) affecting the loop counter.
   */
  const std::string counter = "_ru1_" + q.name;
  const std::string block = "_ru2_" + q.name;
  const std::string handle = "ru_" + q.name;

  /* Calculate the width of the loop counter type. If we don't have a proper
   * type, we just default to a width that covers the full value_t for now.
   */
  std::string width;
  if (q.type == nullptr) {
    width = "(sizeof(value_t) * 8 + 1)";
  } else {
    width = "SIZE_C(" + q.type->width().get_str() + ")";
  }

  /* Write out the step in advance. We generate this here, rather than inline so
   * as to avoid evaluating it twice (once here and then once in the
   * generate_quantifier_footer) as it may contain side effects.
   */
  out
    << "{\n"
    << "  const value_t step = ";
  if (q.step == nullptr) {
    out << "VALUE_C(1)";
  } else {
    generate_rvalue(out, *q.step);
  }
  out << ";\n";

  // Similar for the upper and lower bounds.

  out << "  const value_t lb = ";
  if (q.type == nullptr) {
    assert(q.from != nullptr);
    generate_rvalue(out, *q.from);
  } else {
    out << q.type->lower_bound();
  }
  out << ";\n";

  out << "  const value_t ub = ";
  if (q.type == nullptr) {
    assert(q.to != nullptr);
    generate_rvalue(out, *q.to);
  } else {
    out << q.type->upper_bound();
  }
  out << ";\n";

  out << "  for (value_t " << counter << " = lb; " << counter << " <= ub; "
    << counter << " += step) {\n"
    << "    uint8_t " << block << "[BITS_TO_BYTES(" << width << ")] = { 0 };\n"
    << "    struct handle " << handle << " = { .base = " << block
      << ", .offset = 0, .width = " << width << " };\n"
    << "    handle_write(rule_name, \"" + escape(q.to_string()) + "\", s, lb, ub, "
      << handle << ", " << counter << ");\n";
}

void generate_quantifier_footer(std::ostream &out, const Quantifier &q) {

  const std::string counter = "_ru1_" + q.name;

  out
    << "    if (MAX(value_t) - step < ub && " << counter
      << " > MAX(value_t) - step) {\n"
    << "      break;\n"
    << "    }\n"
    << "  }\n"
    << "}\n";
}
