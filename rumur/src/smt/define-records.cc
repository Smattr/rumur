#include "define-records.h"
#include "solver.h"
#include "translate.h"
#include "typeexpr-to-smt.h"
#include <cstddef>
#include <rumur/rumur.h>
#include <string>

using namespace rumur;

namespace smt {

namespace {
class Definer : public ConstTypeTraversal {

private:
  Solver *solver;

public:
  explicit Definer(Solver &solver_) : solver(&solver_) {}

  void visit_array(const Array &n) final {
    // the index has to be a simple type, so can't contain any records, but the
    // element might
    dispatch(*n.element_type);
  }

  void visit_enum(const Enum &) final {
    // nothing to do
  }

  void visit_range(const Range &) final {
    // nothing to do
  }

  void visit_record(const Record &n) final {

    // first, make sure we define any records that are children of this one
    for (const Ptr<VarDecl> &field : n.fields)
      dispatch(*field->type);

    // now we're ready to define this one itself

    // synthesise a name for the SMT type
    const std::string name = mangle("", n.unique_id);

    // a name for the constructor (we never actually use this)
    const std::string ctor = "mk" + name;

    // declare the type
    *solver << "(declare-datatypes () ((" << name << " (" << ctor;
    for (const Ptr<VarDecl> &field : n.fields) {
      const std::string fname = name + "_" + field->name;
      *solver << " (" << fname << " " << typeexpr_to_smt(*field->type) << ")";
    }
    *solver << "))))\n";
  }

  void visit_typeexprid(const TypeExprID &) final {
    // nothing to do
  }

  void visit_scalarset(const Scalarset &) final {
    // nothing to do
  }
};
} // namespace

void define_records(Solver &solver, const TypeExpr &type) {
  Definer definer(solver);
  definer.dispatch(type);
}

} // namespace smt
