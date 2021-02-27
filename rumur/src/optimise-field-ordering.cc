#include "optimise-field-ordering.h"
#include "log.h"
#include <cstddef>
#include <gmpxx.h>
#include <rumur/rumur.h>
#include <string>
#include <unordered_map>
#include <vector>

using namespace rumur;

// is this value a power of 2?
static bool is_onehot(mpz_class v) {

  if (v == 0)
    return false;

  return mpz_popcount(v.get_mpz_t()) == 1;
}

// compare two fields based on size
static bool comp(const Ptr<VarDecl> &a, const Ptr<VarDecl> &b) {

  mpz_class width_a = a->type->width();
  mpz_class width_b = b->type->width();

  // zero-width fields trump anything else
  if (width_a == 0) {
    return true;
  } else if (width_b == 0) {
    return false;
  }

  // power-of-2 fields trump non-power-of-2 fields
  if (is_onehot(width_a) && !is_onehot(width_b))
    return true;
  if (!is_onehot(width_a) && is_onehot(width_b))
    return false;

  // otherwise, order based on larger fields first
  return width_a > width_b;
}

// sort a collection of fields
static void sort(std::vector<Ptr<VarDecl>> &fields) {
  std::sort(fields.begin(), fields.end(), comp);
}

// extract a list of the names of fields within a list
static std::vector<std::string>
get_names(const std::vector<Ptr<VarDecl>> &decls) {
  std::vector<std::string> r;
  for (const Ptr<VarDecl> &d : decls) {
    if (auto f = dynamic_cast<const VarDecl *>(d.get()))
      r.push_back(f->name);
  }
  return r;
}

// generate debug output if a list of fields has changed
static void notify_changes(const std::vector<std::string> &original,
                           const std::vector<Ptr<VarDecl>> &sorted) {

  // extract the current order of the fields
  const std::vector<std::string> current = get_names(sorted);

  // if this has changed since the original, debug-print the changes
  if (original != current) {

    *debug << "sorted fields {";
    {
      std::string sep;
      for (const std::string &f : original) {
        *debug << sep << f;
        sep = ", ";
      }
    }
    *debug << "} -> {";
    {
      std::string sep = "";
      for (const std::string &f : current) {
        *debug << sep << f;
        sep = ", ";
      }
    }
    *debug << "}\n";
  }
}

// a traversal that reorders fields
namespace {
class Reorderer : public Traversal {

public:
  // The default traversal does not descend into the referent of ExprIDs.
  // However, we do need to because they may have a copy of a record type whose
  // fields we have reordered. We need to also encounter the copy and reorder
  // its fields the same way.
  void visit_exprid(ExprID &n) final { dispatch(*n.value); }

  void visit_model(Model &n) final {

    // first act on our children
    for (Ptr<Node> &c : n.children)
      dispatch(*c);

    // extract out the VarDecls
    std::vector<Ptr<VarDecl>> vars;
    for (Ptr<Node> &c : n.children) {
      if (auto v = dynamic_cast<VarDecl *>(c.get())) {
        auto vp = Ptr<VarDecl>::make(*v);
        vars.push_back(vp);
      }
    }

    const std::vector<std::string> original = get_names(vars);

    // sort the variables
    sort(vars);

    notify_changes(original, vars);

    // the offset of each variable within the model state is now inaccurate, so
    // calculate the new VarDecl -> offset mapping
    mpz_class offset = 0;
    std::unordered_map<std::string, mpz_class> offsets;
    for (Ptr<VarDecl> &v : vars) {
      offsets[v->name] = offset;
      offset += v->type->width();
    }

    // apply these updated offsets to the original VarDecls
    for (Ptr<Node> &c : n.children) {
      if (auto v = dynamic_cast<VarDecl *>(c.get()))
        v->offset = offsets[v->name];
    }
  }

  void visit_record(Record &n) final {

    // first act on our children
    for (Ptr<VarDecl> &f : n.fields)
      dispatch(*f);

    const std::vector<std::string> original = get_names(n.fields);

    // sort the fields of the record itself
    sort(n.fields);

    notify_changes(original, n.fields);
  }

  // like ExprIDs, we also need to force descending into TypeExprID’s referents
  void visit_typeexprid(TypeExprID &n) final { dispatch(*n.referent); }
};
} // namespace

void optimise_field_ordering(Model &m) {
  Reorderer r;
  r.dispatch(m);
}
