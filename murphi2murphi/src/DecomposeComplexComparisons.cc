#include <cstddef>
#include <cassert>
#include "DecomposeComplexComparisons.h"
#include <rumur/rumur.h>
#include <sstream>
#include "Stage.h"
#include <string>

using namespace rumur;

DecomposeComplexComparisons::DecomposeComplexComparisons(Stage &next_):
  IntermediateStage(next_) { }

void DecomposeComplexComparisons::visit_eq(const Eq &n) {
  rewrite(n, true);
}

void DecomposeComplexComparisons::visit_neq(const Neq &n) {
  rewrite(n, false);
}

static std::string explode(const std::string &prefix_a,
    const std::string &prefix_b, const std::string &stem, const TypeExpr &type,
    bool is_eq) {

  std::ostringstream buf;

  // if this is a simple type, we have bottomed out in something we can directly
  // compare
  if (type.is_simple()) {
    buf << prefix_a << stem << (is_eq ? " = " : " != ") << prefix_b << stem;
    return buf.str();
  }

  const Ptr<TypeExpr> t = type.resolve();

  // if this is a record, join together a comparison of each of its fields
  if (auto r = dynamic_cast<const Record*>(t.get())) {
    std::string sep;
    for (const Ptr<VarDecl> &f : r->fields) {
      buf << sep << "("
        << explode(prefix_a, prefix_b, stem + "." + f->name, *f->type, is_eq)
        << ")";
      sep = is_eq ? " & " : " | ";
    }
    return buf.str();
  }

  auto a = dynamic_cast<const Array*>(t.get());
  assert(a != nullptr && "non-record, non-array encountered when decomposing "
    "complex expression");

  const std::string binder = is_eq ? "forall" : "exists";

  // FIXME: we need to generate a unique variable for the binder here
  buf << binder << " x: " << a->index_type->to_string() << " do "
    << explode(prefix_a, prefix_b, stem + "[x]", *a->element_type, is_eq)
    << " end" << binder;
  return buf.str();
}

void DecomposeComplexComparisons::rewrite(const EquatableBinaryExpr &n,
    bool is_eq) {

  // if this is a comparison of simple types, we can let it pass through
  const Ptr<TypeExpr> t = n.lhs->type();
  if (t->is_simple()) {

    assert(n.rhs->type()->is_simple()
      && "comparison of simple type to complex type");

    next.dispatch(n);
    return;
  }

  // if either side of the comparison has side effects, we cannot decompose it
  // because emitting the stem multiple times will cause the side effect(s) to
  // repeat
  if (!n.lhs->is_pure())
    throw Error("cannot decompose complex comparison because the left hand "
      "side has side effects", n.lhs->loc);
  if (!n.rhs->is_pure())
    throw Error("cannot decompose complex comparison because the right hand "
      "side has side effects", n.rhs->loc);

  // synchronise output to the start of this comparison
  top->sync_to(n);

  // skip this comparison itself
  top->skip_to(n.loc.end);

  // write a decomposed version of the comparison
  *top << explode(n.lhs->to_string(), n.rhs->to_string(), "", *t, is_eq);
}
