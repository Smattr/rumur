#include "ValueType.h"
#include "log.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <limits.h>
#include <rumur/rumur.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace rumur;

namespace {

class BoundsFinder : public ConstTraversal {

public:
  mpz_class min = 0;
  mpz_class max = 0;

private:
  void increase_max(const mpz_class &new_value, const std::string &cause) {
    *debug << "increasing maximum numerical bound to " << new_value
           << " due to \"" << cause << "\"\n";
    max = new_value;
  }

  void decrease_min(const mpz_class &new_value, const std::string &cause) {
    *debug << "decreasing minimum numerical bound to " << new_value
           << " due to \"" << cause << "\"\n";
    min = new_value;
  }

public:
  void visit_enum(const Enum &n) final {
    if (n.members.size() > max)
      increase_max(n.members.size(), n.to_string());
  }

  /* We explicitly handle negative expressions because the Rumur AST sees them
   * as something compound, but users tend to think of them as an atom. For
   * example, if a user writes "-1" they expect the automatically derived type
   * will be able to contain -1. If we did not handle Negate specifically, this
   * analysis would only look at the inner "1" and conclude a narrower range
   * than was intuitive to the user.
   */
  void visit_negative(const Negative &n) final {
    if (auto l = dynamic_cast<const Number *>(&*n.rhs)) {
      mpz_class v = -l->value;
      if (v < min)
        decrease_min(v, n.to_string());
      if (v > max)
        increase_max(v, n.to_string());
    }
    dispatch(*n.rhs);
  }

  void visit_number(const Number &n) final {
    if (n.value < min)
      decrease_min(n.value, n.to_string());
    if (n.value > max)
      increase_max(n.value, n.to_string());
  }

  // we override visit_quantifier in order to also descend into the quantifier’s
  // decl that the generic traversal logic assumes you do not want to do
  void visit_quantifier(const Quantifier &n) final {
    if (n.type != nullptr)
      dispatch(*n.type);
    if (n.from != nullptr)
      dispatch(*n.from);
    if (n.to != nullptr)
      dispatch(*n.to);
    if (n.step != nullptr)
      dispatch(*n.step);
    dispatch(*n.decl);
  }

  void visit_range(const Range &n) final {
    if (n.min->constant()) {
      mpz_class m = n.min->constant_fold();
      if (m < min)
        decrease_min(m, n.min->to_string());
      if (m > max)
        increase_max(m, n.min->to_string());
    }
    if (n.max->constant()) {
      mpz_class m = n.max->constant_fold();
      if (m < min)
        decrease_min(m, n.max->to_string());
      if (m > max)
        increase_max(m, n.max->to_string());
    }
  }

  void visit_scalarset(const Scalarset &n) final {
    if (n.bound->constant()) {
      mpz_class m = n.bound->constant_fold();
      if (m < min)
        decrease_min(m, n.bound->to_string());
      if (m > max)
        increase_max(m, n.bound->to_string());
    }
  }
};

} // namespace

static const std::unordered_map<std::string, ValueType> types = {
    // clang-format off
    { "int8_t",   { "int_fast8_t",   "INT_FAST8_MIN",      "INT_FAST8_MAX",   "INT8_C",   "PRIdFAST8",  mpz_class(std::to_string(INT8_MIN)),  mpz_class(std::to_string(INT8_MAX))   } },
    { "uint8_t",  { "uint_fast8_t",  "((uint_fast8_t)0)",  "UINT_FAST8_MAX",  "UINT8_C",  "PRIuFAST8",  0,                                    mpz_class(std::to_string(UINT8_MAX))  } },
    { "int16_t",  { "int_fast16_t",  "INT_FAST16_MIN",     "INT_FAST16_MAX",  "INT16_C",  "PRIdFAST16", mpz_class(std::to_string(INT16_MIN)), mpz_class(std::to_string(INT16_MAX))  } },
    { "uint16_t", { "uint_fast16_t", "((uint_fast16_t)0)", "UINT_FAST16_MAX", "UINT16_C", "PRIuFAST16", 0,                                    mpz_class(std::to_string(UINT16_MAX)) } },
    { "int32_t",  { "int_fast32_t",  "INT_FAST32_MIN",     "INT_FAST32_MAX",  "INT32_C",  "PRIdFAST32", mpz_class(std::to_string(INT32_MIN)), mpz_class(std::to_string(INT32_MAX))  } },
    { "uint32_t", { "uint_fast32_t", "((uint_fast32_t)0)", "UINT_FAST32_MAX", "UINT32_C", "PRIuFAST32", 0,                                    mpz_class(std::to_string(UINT32_MAX)) } },
    { "int64_t",  { "int_fast64_t",  "INT_FAST64_MIN",     "INT_FAST64_MAX",  "INT64_C",  "PRIdFAST64", mpz_class(std::to_string(INT64_MIN)), mpz_class(std::to_string(INT64_MAX))  } },
    { "uint64_t", { "uint_fast64_t", "((uint_fast64_t)0)", "UINT_FAST64_MAX", "UINT64_C", "PRIuFAST64", 0,                                    mpz_class(std::to_string(UINT64_MAX)) } },
    // clang-format on
};

// a list of the types sorted by estimated expense
static const std::vector<std::string> TYPES = {
    "uint8_t",  "int8_t",  "uint16_t", "int16_t",
    "uint32_t", "int32_t", "uint64_t", "int64_t"};

// find an unsigned type that can contain the given range shifted to be 1-based
static const ValueType raw_type(const mpz_class &min, const mpz_class &max) {

  assert(min <= max && "reversed bounds in raw_type()");

  mpz_class limit = max - min + 1;

  for (const std::string &t : TYPES) {
    const ValueType &vt = types.at(t);

    // skip signed types
    if (vt.min < 0)
      continue;

    if (vt.max >= limit)
      return vt;
  }

  throw std::runtime_error("no supported unsigned type is wide enough to "
                           "contain enough values to cover the range [" +
                           min.get_str() + ", " + max.get_str() + "]");
}

std::pair<ValueType, ValueType> get_value_type(const std::string &name,
                                               const Model &m) {

  if (name == "auto") {

    // find least and greatest numerical values needed for this model
    BoundsFinder bf;
    bf.dispatch(m);

    // find the first type that satisfies these
    for (const std::string &t : TYPES) {
      *debug << "considering type " << t << "...\n";
      const ValueType &vt = types.at(t);
      if (vt.min <= bf.min && vt.max >= bf.max) {
        *info << "using numerical type " << t << " as value type\n";
        return {vt, raw_type(bf.min, bf.max)};
      }
    }

    throw std::runtime_error("model's numerical bounds are [" +
                             bf.min.get_str() + ", " + bf.max.get_str() +
                             "] which no supported type contains");
  }

  auto it = types.find(name);
  if (it == types.end())
    throw std::runtime_error("unknown type " + name);

  const ValueType &vt = it->second;

  return {vt, raw_type(vt.min, vt.max)};
}
