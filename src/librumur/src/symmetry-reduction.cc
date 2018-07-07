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
 * many times is a scalarset used in this type?"
 */
static unsigned interdependence(const TypeExpr &t) {

  if (auto r = dynamic_cast<const Record*>(&t)) {
    unsigned s = 0;
    for (const VarDecl *f : r->fields)
      s += interdependence(*f->type);
    return s;
  }

  if (auto a = dynamic_cast<const Array*>(&t))
    return interdependence(*a->index_type) + interdependence(*a->element_type);

  if (auto s = dynamic_cast<const Scalarset*>(&t))
    return 1;

  if (auto i = dynamic_cast<const TypeExprID*>(&t))
    return interdependence(*i->referent);

  return 0;
}

/* A "pivot," a collection of state components that we can sort at runtime to
 * provide a canonical ordering for a given scalarset type.
 */
namespace { struct Pivot {

  struct Component {
    size_t offset;
    const VarDecl *decl;
    unsigned interdependence;
  };

  const Model *model;
  const TypeDecl *type;
  std::vector<Component> components;

  Pivot(const Model &m, const TypeDecl &type_): model(&m), type(&type_) { }
  Pivot(const Pivot&) = default;
  Pivot(Pivot&&) = default;
  Pivot &operator=(Pivot&&) = default;
  Pivot &operator=(const Pivot&) = default;

  bool is_eligible_component(const VarDecl &v) const {

    // First, fully resolve the type in case it is a typedef.
    const TypeExpr *vtype = v.type->resolve();

    // For now, we only consider using arrays as pivot components...
    auto a = dynamic_cast<const Array*>(vtype);
    if (a == nullptr)
      return false;

    // ...and only those indexed on our type.
    auto t = dynamic_cast<const TypeExprID*>(a->index_type);
    if (t == nullptr)
      return false;
    if (t->name != type->name)
      return false;

    return true;
  }

  // Add a component to this pivot.
  void add_component(size_t offset, const VarDecl &v) {

    /* Find the representative type against which we'll score this VarDecl on
     * interdependence. E.g. in the case of a scalarset-indexed array, we score
     * the VarDecl based on the element type alone because we know the
     * (positive) effect of the index type.
     */
    const TypeExpr *vtype = v.type->resolve();
    if (auto a = dynamic_cast<const Array*>(vtype))
      vtype = a->element_type;

    // Assess how much interaction this component has with other scalarsets.
    unsigned i = interdependence(*vtype);

    Component c = { offset, &v, i };

    /* Insert the component in order of increasing interdependence so that we
     * can output more optimal pivot code eventually.
     */
    for (auto it = components.begin(); it != components.end(); it++) {
      if (c.interdependence <= it->interdependence) {
        components.insert(it, c);
        return;
      }
    }

    /* The element to insert scored higher than every existing component. Insert
     * at the end.
     */
    components.emplace_back(c);
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

  // Construct a pivot for the given scalarset declaration.
  static Pivot derive(const Model &m, const TypeDecl &t) {
    assert(dynamic_cast<const Scalarset*>(t.value) != nullptr &&
      "non-scalarset typedecl passed to Pivot::derive");

    Pivot p(m, t);

    // Look through the model state to find suitable components.
    size_t offset = 0;
    for (const Decl *d : m.decls) {
      if (auto v = dynamic_cast<const VarDecl*>(d)) {
        p.consider_component(offset, *v);
        offset += v->type->width();
      }
    }

    return p;
  }

  /* Score this pivot as a whole based on its components. This is a measure of
   * how much reshuffling based on this pivot will degrade the ability of other
   * pivots. As with 'interdependence,' lower is better.
   */
  unsigned interference(void) const {
    unsigned s = 0;
    for (const Component &c : components)
      s += c.interdependence;
    return s;
  }

  void generate_definitions(std::ostream &out) const {
    auto s = dynamic_cast<const Scalarset&>(*type->value);
    int64_t b = s.bound->constant_fold();

      /* An array indicating how to sort the given scalarset. For example, if we
       * have a scalarset(4), this array may end up as { 3, 1, 0, 2 }. This
       * would indicate any other scalarset state data encountered should map 0
       * to 3, 1 to itself, 2 to 0, and 3 to 2.
       */
    out << "  size_t schedule_" << type->name << "[" << b << "] "
      << "__attribute__((unused));\n";
  }

}; }

void generate_canonicalise(const Model &m, std::ostream &out) {

  // Write the function header
  out
    << "static void state_canonicalise(struct state *s "
      << "__attribute__((unused))) {\n";

  // Find the named scalarset types.
  std::vector<const TypeDecl*> ss = find_scalarsets(m);
  *log.info << "symmetry reduction: " << ss.size() << " eligible scalarset "
    "types\n";

  /* Derive a pivot for each one, keeping the list sorted by ascending
   * interference.
   */
  std::vector<Pivot> pivots;
  for (const TypeDecl *t : ss) {
    Pivot p = Pivot::derive(m, *t);
    *log.debug << __func__ << ":" << __LINE__ << ": symmetry reduction: pivot "
      << "for scalarset type " << t->name << " has an interference score of "
      << p.interference() << (p.interference() == 0 ? " (perfect)" : "")
      << "\n";
    bool found = false;
    for (auto it = pivots.begin(); it != pivots.end(); it++) {
      if (p.interference() <= it->interference()) {
        pivots.insert(it, p);
        found = true;
        break;
      }
    }
    if (!found)
      pivots.push_back(p);
  }

  // Output code to perform the actual canonicalisation
  for (const Pivot &p : pivots)
    p.generate_definitions(out);

  out << "}";
}

}
