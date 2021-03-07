#include "generate.h"
#include <cassert>
#include <cstddef>
#include <iostream>
#include <rumur/rumur.h>
#include <string>

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

  // open a scope to allow us to use the names 'lb', 'ub', and 'step' without
  // worrying about collisions
  out << "{\n";

  // Write out the upper bound, lower bound, and step in advance. We generate
  // these here, rather than inline so as to avoid evaluating them twice (once
  // here and then once in the generate_quantifier_footer) as they may contain
  // side effects.

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

  out << "  const raw_value_t step = (raw_value_t)";
  if (q.step == nullptr) {
    out << "(ub >= lb ? 1 : -1)";
  } else {
    generate_rvalue(out, *q.step);
  }
  out << ";\n";

  // if the step was not a generation-time constant, it is possible that it
  // could work out to be 0 at runtime resulting in an infinite loop
  out << "  if (step == 0) {\n"
      << "    error(s, \"infinite loop due to step being 0\");\n"
      << "  }\n";

  // it is also possible we find the iteration goes the wrong way
  out << "#ifdef __clang__\n"
      << "  #pragma clang diagnostic push\n"
      << "  #pragma clang diagnostic ignored "
         "\"-Wtautological-unsigned-zero-compare\"\n"
      << "#elif defined(__GNUC__)\n"
      << "  #pragma GCC diagnostic push\n"
      << "  #pragma GCC diagnostic ignored \"-Wtype-limits\"\n"
      << "#endif\n"
      << "  if ((ub > lb && (value_t)step < 0) ||\n"
      << "      (ub < lb && (value_t)step > 0)) {\n"
      << "#ifdef __clang__\n"
      << "  #pragma clang diagnostic pop\n"
      << "#elif defined(__GNUC__)\n"
      << "  #pragma GCC diagnostic pop\n"
      << "#endif\n"
      << "    error(s, \"infinite loop due to step being in the wrong "
         "direction\");\n"
      << "  }\n";

  // Type lower bound, as distinct from the iteration lower bound.
  // bounds. References to this quantified variable will use these when
  // unpacking its compressed representation, so we need to offset the values we
  // store from it.
  const std::string lower = q.decl->type->lower_bound();
  const std::string upper = q.decl->type->upper_bound();

  out << "#if !defined(__clang__) && defined(__GNUC__)\n"
      << "  #pragma GCC diagnostic push\n"
      << "  #pragma GCC diagnostic ignored \"-Wtype-limits\"\n"
      << "#endif\n"
      << "  ASSERT(lb >= " << lower << " && lb <= " << upper
      << " && "
         "\"iteration lower bound exceeds type limits\");\n"
      << "  ASSERT(ub >= " << lower << " && ub <= " << upper
      << " && "
         "\"iteration upper bound exceeds type limits\");\n"
      << "#if !defined(__clang__) && defined(__GNUC__)\n"
      << "  #pragma GCC diagnostic pop\n"
      << "#endif\n";

// shorthands for converting between value_t and raw_value_t when we know the
// value we have is in range
#define V_TO_RV(x)                                                             \
  ("((raw_value_t)((raw_value_t)(((raw_value_t)" + std::string(x) +            \
   ") + (raw_value_t)1) - (raw_value_t)(" + q.decl->type->lower_bound() +      \
   ")))")
#define RV_TO_V(x)                                                             \
  ("((value_t)(" + std::string(x) + " - (raw_value_t)1 + (raw_value_t)(" +     \
   q.decl->type->lower_bound() + ")))")

  // construct the pieces of our for-loop header
  const std::string init = "raw_value_t " + counter + " = " + V_TO_RV("lb");
  const std::string cond = counter + " == " + V_TO_RV("lb") +
                           " || "
                           "(lb < ub && " +
                           RV_TO_V(counter) +
                           " <= ub) || "
                           "(lb > ub && " +
                           RV_TO_V(counter) + " >= ub)";
  const std::string inc = counter + " += step";

  out << "  for (" << init << "; " << cond << "; " << inc << ") {\n"
      << "    uint8_t " << block << "[BITS_TO_BYTES(" << width
      << ")] = { 0 };\n"
      << "    struct handle " << handle << " = { .base = " << block
      << ", .offset = 0, .width = " << width << " };\n"
      << "    handle_write_raw(s, " << handle << ", " << counter << ");\n";
}

void generate_quantifier_footer(std::ostream &out, const Quantifier &q) {

  const std::string counter = "_ru1_" + q.name;

  // is this a loop whose last iteration will result in a numeric overflow?
  std::string will_overflow = RV_TO_V("max_ - step") + " < ub";

  // are we on the last iteration?
  std::string last_iteration = counter + " > RAW_VALUE_MAX - step";

  // does this loop count up?
  const std::string up_count = "ub >= lb && (value_t)step > 0";

  out << "    /* If this iteration runs right up to the type limits, the last\n"
      << "     * increment will overflow and fail to terminate, so we guard\n"
      << "     * against that here.\n"
      << "     */\n"
      << "    {\n"
      << "      /* GCC issues spurious -Wsign-compare warnings when doing\n"
      << "       * subtraction on raw_value_t literals, so we suppress these\n"
      << "       * by indirecting via this local constant. For more "
         "information:\n"
      << "       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341\n"
      << "       */\n"
      << "      const raw_value_t max_ = RAW_VALUE_MAX;\n"
      << "      if (" << up_count << " && " << will_overflow << " && "
      << last_iteration << ") {\n"
      << "        break;\n"
      << "      }\n"
      << "    }\n";

  // do the same for if it is a down-counting loop
  will_overflow = RV_TO_V("min_ - step") + " > ub";
  last_iteration = RV_TO_V(counter) + " < " + RV_TO_V("RAW_VALUE_MIN - step");
  const std::string down_count = "ub <= lb && (value_t)step < 0";

  out << "#ifdef __clang__\n"
      << "  #pragma clang diagnostic push\n"
      << "  #pragma clang diagnostic ignored "
         "\"-Wtautological-unsigned-zero-compare\"\n"
      << "#elif defined(__GNUC__)\n"
      << "  #pragma GCC diagnostic push\n"
      << "  #pragma GCC diagnostic ignored \"-Wtype-limits\"\n"
      << "  #pragma GCC diagnostic ignored \"-Wsign-compare\"\n"
      << "#endif\n"
      << "    {\n"
      << "      /* see above explanation of why we use an extra constant here "
         "*/\n"
      << "      const raw_value_t min_ = RAW_VALUE_MIN;\n"
      << "      if (" << down_count << " && " << will_overflow << " && "
      << last_iteration << ") {\n"
      << "#ifdef __clang__\n"
      << "  #pragma clang diagnostic pop\n"
      << "#elif defined(__GNUC__)\n"
      << "  #pragma GCC diagnostic pop\n"
      << "#endif\n"
      << "        break;\n"
      << "      }\n"
      << "    }\n";

  out << "  }\n"
      << "}\n";
}

#undef V_TO_RV
#undef RV_TO_V
