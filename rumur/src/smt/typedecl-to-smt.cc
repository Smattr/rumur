#include <cstddef>
#include "define-enum-members.h"
#include "except.h"
#include <rumur/rumur.h>
#include "solver.h"
#include "typedecl-to-smt.h"
#include "../utils.h"

using namespace rumur;

namespace smt {

namespace { class Translator : public ConstTypeTraversal {

 private:
  Solver *solver;

 public:
  Translator(Solver &solver_):
    solver(&solver_) { }

  void visit_array(const Array&) final {
    throw Unsupported();
  }

  void visit_enum(const Enum &n) final {
    // we need to emit the members as values
    define_enum_members(*solver, n);
  }

  void visit_range(const Range&) final {
    /* we can ignore ranges as their constraints are emitted for a VarDecl that
     * is declared as an Int
     */
  }

  void visit_record(const Record&) final {
    throw Unsupported();
  }

  void visit_scalarset(const Scalarset&) final {
    // we can ignored scalarsets for the same reason as ranges
  }

  void visit_typeexprid(const TypeExprID&) final {
    // typedecl of a typedecl
  }
}; }

void typedecl_to_smt(Solver &solver, const TypeDecl &typedecl) {
  Translator translator(solver);
  translator.dispatch(*typedecl.value);
}

}
