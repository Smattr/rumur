#include "prints-scalarsets.h"
#include "../../common/isa.h"
#include <cstddef>
#include <rumur/rumur.h>

using namespace rumur;

// a traversal that recognises types containing scalarset references
namespace {
class ContainsScalarset : public ConstTraversal {

public:
  bool result = false;

  void visit_typeexprid(const TypeExprID &n) final {
    const Ptr<TypeExpr> t = n.referent->value->resolve();
    result |= isa<Scalarset>(t);

    // we need to descend into the reference in case it is, e.g., an array type
    // with a scalarset index
    dispatch(*n.referent);
  }
};
} // namespace

static bool contains_scalarset(const TypeExpr &t) {
  ContainsScalarset c;
  c.dispatch(t);
  return c.result;
}

// a traversal that recognises put statements that rely on scalarsets
namespace {
class PutsScalarset : public ConstTraversal {

public:
  bool result = false;

  void visit_put(const Put &n) final {
    // does this statement rely on a scalarset type?
    if (n.expr != nullptr)
      result |= contains_scalarset(*n.expr->type());
  }
};
} // namespace

bool prints_scalarsets(const Model &m) {
  PutsScalarset p;
  p.dispatch(m);
  return p.result;
}
