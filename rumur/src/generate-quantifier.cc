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

  // Calculate the width of the loop counter type. Use the VarDecl that
  // references to this variable will be referring to.
  std::string width = "((size_t)" + q.decl->type->width().get_str() + "ull)";

  /* Write out the step in advance. We generate this here, rather than inline so
   * as to avoid evaluating it twice (once here and then once in the
   * generate_quantifier_footer) as it may contain side effects.
   */
  out
    << "{\n"
    << "  const raw_value_t step = (raw_value_t)";
  if (q.step == nullptr) {
    out << "1";
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

  // Type lower bound, as distinct from the iteration lower bound.
  // bounds. References to this quantified variable will use these when
  // unpacking its compressed representation, so we need to offset the values we
  // store from it.
  const std::string lower = q.decl->type->lower_bound();
  const std::string upper = q.decl->type->upper_bound();

  out
    << "#ifdef __clang__\n"
    << "  #pragma clang diagnostic push\n"
    << "  #pragma clang diagnostic ignored \"-Wtautological-compare\"\n"
    << "#elif defined(__GNUC__)\n"
    << "  #pragma GCC diagnostic push\n"
    << "  #pragma GCC diagnostic ignored \"-Wtype-limits\"\n"
    << "#endif\n"
    << "  ASSERT(lb >= " << lower << " && "
      "\"iteration lower bound exceeds type limits\");\n"
    << "  ASSERT(ub <= " << upper << " && "
      "\"iteration upper bound exceeds type limits\");\n"
    << "#ifdef __clang__\n"
    << "  #pragma clang diagnostic pop\n"
    << "#elif defined(__GNUC__)\n"
    << "  #pragma GCC diagnostic pop\n"
    << "#endif\n";

// shorthands for converting between value_t and raw_value_t when we know the
// value we have is in range
#define V_TO_RV(x) ("(((raw_value_t)" + std::string(x) + \
  ") + (raw_value_t)1 - (raw_value_t)(" + q.decl->type->lower_bound() + "))")
#define RV_TO_V(x) ("((value_t)(" + std::string(x) + \
  " - (raw_value_t)1 + (raw_value_t)(" + q.decl->type->lower_bound() + ")))")

  // construct the pieces of our for-loop header
  const std::string init = "raw_value_t " + counter + " = " + V_TO_RV("lb");
  const std::string cond = RV_TO_V(counter) + " <= ub";
  const std::string inc = counter + " += step";

  out
    << "  for (" << init << "; " << cond << "; " << inc << ") {\n"
    << "    uint8_t " << block << "[BITS_TO_BYTES(" << width << ")] = { 0 };\n"
    << "    struct handle " << handle << " = { .base = " << block
      << ", .offset = 0, .width = " << width << " };\n"
    << "    handle_write_raw(" << handle << ", " << counter << ");\n";
}

void generate_quantifier_footer(std::ostream &out, const Quantifier &q) {

  const std::string counter = "_ru1_" + q.name;

  // is this a loop whose last iteration will result in a numeric overflow?
  const std::string will_overflow = RV_TO_V("RAW_VALUE_MAX - step") + " < ub";

  // are we on the last iteration?
  const std::string last_iteration = counter + " > RAW_VALUE_MAX - step";

  out
    << "    /* If this iteration runs right up to the type limits, the last\n"
    << "     * increment will overflow and fail to terminate, so we guard\n"
    << "     * against that here.\n"
    << "     */\n"
    << "    if (" <<  will_overflow << " && " << last_iteration << ") {\n"
    << "      break;\n"
    << "    }\n"
    << "  }\n"
    << "}\n";
}

#undef V_TO_RV
#undef RV_TO_V
