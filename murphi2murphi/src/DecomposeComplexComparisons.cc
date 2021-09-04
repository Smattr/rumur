#include "DecomposeComplexComparisons.h"
#include "Stage.h"
#include <cassert>
#include <climits>
#include <cstddef>
#include <rumur/rumur.h>
#include <sstream>
#include <string>
#include <unordered_set>

using namespace rumur;

DecomposeComplexComparisons::DecomposeComplexComparisons(Stage &next_)
    : IntermediateStage(next_) {}

void DecomposeComplexComparisons::visit_eq(const Eq &n) { rewrite(n, true); }

void DecomposeComplexComparisons::visit_neq(const Neq &n) { rewrite(n, false); }

// find all identifiers used within a given expression
static std::unordered_set<std::string> find_ids(const Expr &e) {

  // a traversal that collects ExprIDs
  class ExprIDFinder : public ConstTraversal {

  public:
    std::unordered_set<std::string> ids;

    void visit_exprid(const ExprID &n) final { (void)ids.insert(n.id); }
  };

  // use this to find all contained ExprIDs
  ExprIDFinder finder;
  finder.dispatch(e);

  return finder.ids;
}

// create a new identifier, unique with respect to the given identifiers
static std::string make_id(std::unordered_set<std::string> &ids) {
  // arbitrary prefix
  std::string v = "i";
  for (size_t i = 0;; i++) {
    std::string candidate = v + std::to_string(i);
    auto res = ids.insert(candidate);
    if (res.second)
      return candidate;
    assert(i != SIZE_MAX && "exhausted variable space");
  }
  __builtin_unreachable();
}

static std::string explode(std::unordered_set<std::string> &ids,
                           const std::string &prefix_a,
                           const std::string &prefix_b, const std::string &stem,
                           const TypeExpr &type, bool is_eq) {

  std::ostringstream buf;

  // if this is a simple type, we have bottomed out in something we can directly
  // compare
  if (type.is_simple()) {
    buf << prefix_a << stem << (is_eq ? " = " : " != ") << prefix_b << stem;
    return buf.str();
  }

  const Ptr<TypeExpr> t = type.resolve();

  // if this is a record, join together a comparison of each of its fields
  if (auto r = dynamic_cast<const Record *>(t.get())) {
    std::string sep;
    for (const Ptr<VarDecl> &f : r->fields) {
      const std::string new_stem = stem + "." + f->name;
      buf << sep << "("
          << explode(ids, prefix_a, prefix_b, new_stem, *f->type, is_eq) << ")";
      sep = is_eq ? " & " : " | ";
    }
    return buf.str();
  }

  auto a = dynamic_cast<const Array *>(t.get());
  assert(a != nullptr && "non-record, non-array encountered when decomposing "
                         "complex expression");

  const std::string binder = is_eq ? "forall" : "exists";

  // invent an id for the index in the binder
  const std::string i = make_id(ids);

  const std::string new_stem = stem + "[" + i + "]";
  buf << binder << " " << i << ": " << a->index_type->to_string() << " do "
      << explode(ids, prefix_a, prefix_b, new_stem, *a->element_type, is_eq)
      << " end" << binder;

  return buf.str();
}

void DecomposeComplexComparisons::rewrite(const EquatableBinaryExpr &n,
                                          bool is_eq) {

  // if this is a comparison of simple types, we can let it pass through
  const Ptr<TypeExpr> t = n.lhs->type();
  if (t->is_simple()) {

    assert(n.rhs->type()->is_simple() &&
           "comparison of simple type to complex type");

    next.dispatch(n);
    return;
  }

  // if either side of the comparison has side effects, we cannot decompose it
  // because emitting the stem multiple times will cause the side effect(s) to
  // repeat
  if (!n.lhs->is_pure())
    throw Error("cannot decompose complex comparison because the left hand "
                "side has side effects",
                n.lhs->loc);
  if (!n.rhs->is_pure())
    throw Error("cannot decompose complex comparison because the right hand "
                "side has side effects",
                n.rhs->loc);

  // synchronise output to the start of this comparison
  top->sync_to(n);

  // skip this comparison itself
  top->skip_to(n.loc.end);

  // find any identifiers used in either LHS or RHS
  std::unordered_set<std::string> ids = find_ids(*n.lhs);
  std::unordered_set<std::string> rhs_ids = find_ids(*n.rhs);
  ids.insert(rhs_ids.begin(), rhs_ids.end());

  // write a decomposed version of the comparison
  *top << explode(ids, n.lhs->to_string(), n.rhs->to_string(), "", *t, is_eq);
}
