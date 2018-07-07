#include <cassert>
#include <iostream>
#include <rumur/Decl.h>
#include <rumur/log.h>
#include <rumur/Model.h>
#include <rumur/symmetry-reduction.h>
#include <rumur/traverse.h>
#include <rumur/TypeExpr.h>
#include <utility>
#include <vector>

namespace rumur {

// Find all the named scalarset declarations in a model.
static std::vector<const TypeDecl*> find_scalarsets(const Model &m) {
  std::vector<const TypeDecl*> ss;
  for (const Decl *d : m.decls) {
    if (auto t = dynamic_cast<const TypeDecl*>(d)) {
      if (dynamic_cast<const Scalarset*>(t->value) != nullptr)
        ss.push_back(t);
    }
  }
  return ss;
}

/* Derive a metric for a given type based on its utility as a pivot component
 * (see below). In this scheme lower is better. The score is essentially "how
 * many times is a scalarset used in this type?" We currently only ever call it
 * with scalarset-indexed arrays, so nothing ever scores below 1.
 */
static unsigned score(const TypeExpr &t) {

  if (auto r = dynamic_cast<const Record*>(&t)) {
    unsigned s = 0;
    for (const VarDecl *f : r->fields)
      s += score(*f->type);
    return s;
  }

  if (auto a = dynamic_cast<const Array*>(&t))
    return score(*a->index_type) + score(*a->element_type);

  if (auto s = dynamic_cast<const Scalarset*>(&t))
    return 1;

  if (auto i = dynamic_cast<const TypeExprID*>(&t))
    return score(*i->referent);

  return 0;
}

namespace { struct Pivot {

  const Model &model;
  const std::string name;
  std::vector<std::pair<size_t, const VarDecl*>> components;

  Pivot(const Model &m, const std::string &name_): model(m), name(name_) { }

  bool is_eligible_component(const VarDecl &v) const {

    // For now, we only consider using arrays as pivot components...
    auto a = dynamic_cast<const Array*>(v.type);
    if (a == nullptr)
      return false;

    // ...and only those indexed on our type.
    auto t = dynamic_cast<const TypeExprID*>(a->index_type);
    if (t == nullptr)
      return false;
    if (t->name != name)
      return false;

    return true;
  }

  // Add a component to this pivot.
  void add_component(size_t offset, const VarDecl &v) {

    /* Insert it in order of increasing score so that we can output more optimal
     * pivot code eventually.
     */
    // TODO: We could amortise this by storing scores instead of recomputing them
    unsigned s = score(*v.type);
    for (auto it = components.begin(); it != components.end(); it++) {
      if (score(*it->second->type) > s) {
        components.insert(it, std::make_pair(offset, &v));
        return;
      }
    }

    /* The element to insert scored higher than every existing component. Insert
     * at the end.
     */
    components.emplace_back(std::make_pair(offset, &v));
  }

  // Recursively find and add components
  void consider_component(size_t offset, const VarDecl &v) {

    // If this decl can participate in the pivot, add it and we're done.
    if (is_eligible_component(v)) {
      add_component(offset, v);
      return;
    }

    // If this is a record, consider each of its fields.
    if (auto r = dynamic_cast<const Record*>(v.type)) {
      for (const VarDecl *f : r->fields) {
        consider_component(offset, *f);
        offset += f->type->width();
      }
    }
  }

  static Pivot derive(const Model &m, const TypeDecl &t) {
    assert(dynamic_cast<const Scalarset*>(t.value) != nullptr &&
      "non-scalarset typedecl passed to find_pivot");

    Pivot p(m, t.name);

    size_t offset = 0;
    for (const Decl *d : m.decls) {
      if (auto v = dynamic_cast<const VarDecl*>(d)) {
        p.consider_component(offset, *v);
        offset += v->type->width();
      }
    }

    return p;
  }

}; }

static void generate_pivot_data(const TypeDecl &t, std::ostream &out) {

  auto s = dynamic_cast<const Scalarset&>(*t.value);
  int64_t b = s.bound->constant_fold();

  out
    /* An array indicating how to sort the given scalarset. For example, if we
     * have a scalarset(4), this array may end up as { 3, 1, 0, 2 }. This would
     * indicate any other scalarset state data encountered should map 0 to 3, 1
     * to itself, 2 to 0, and 3 to 2.
     */
    << "  size_t schedule_" << t.name << "[" << b << "] "
      << "__attribute__((unused));\n"

    /* A flag indicating whether we have yet found a "pivot," an array indexed
     * by this scalarset whose contents can be sorted to derive a "schedule"
     * (above). The contents of the schedule should not be considered valid
     * until this flag is true. Conversely if we encounter a sorting candidate
     * and this flag is false, we can consider the candidate the pivot and
     * write the schedule.
     */
    << "  bool pivoted_" << t.name << " __attribute__((unused)) = false;\n";
}

void generate_canonicalise(const Model &m, std::ostream &out) {

  // Write the function header
  out
    << "static void state_canonicalise(struct state *s "
      << "__attribute__((unused))) {\n";

  // Define the schedule and pivot
  std::vector<const TypeDecl*> ss = find_scalarsets(m);
  *log.info << "symmetry reduction: " << ss.size() << " eligible scalarset "
    "types\n";
  for (const TypeDecl *t : ss)
    generate_pivot_data(*t, out);

  std::vector<Pivot> pivots;
  for (const TypeDecl *t : ss)
    pivots.push_back(Pivot::derive(m, *t));

  out << "}";
}

}
