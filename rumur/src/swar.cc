#include "swar.h"
#include <rumur/rumur.h>

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

SwarShape get_swar_type(SwarShape request, const Model &m) {

  // “off”?
  if (request.lanes == 0 && request.lane_width == 0)
    return request;

  // find least and greatest numerical values needed for this model
  BoundsFinder bf;
  bf.dispatch(m);
}
