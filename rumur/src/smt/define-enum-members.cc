#include "define-enum-members.h"
#include "../options.h"
#include "logic.h"
#include "solver.h"
#include "translate.h"
#include <cassert>
#include <cstddef>
#include <gmpxx.h>
#include <rumur/rumur.h>
#include <string>
#include <utility>

using namespace rumur;

namespace smt {

namespace {
class Definer : public ConstTypeTraversal {

private:
  Solver *solver;

public:
  explicit Definer(Solver &solver_) : solver(&solver_) {}

  void visit_array(const Array &n) final {
    // define any enum members that occur in the index or element types
    dispatch(*n.index_type);
    dispatch(*n.element_type);
  }

  void visit_enum(const Enum &n) final {
    // ignore the built-in bool type that the solver already knows
    if (n.is_boolean())
      return;

    // emit the members of the enum as integer constants
    mpz_class index = 0;
    size_t id = n.unique_id + 1;
    for (const std::pair<std::string, location> &member : n.members) {
      assert(id < n.unique_id_limit && "unexpected number of enum members");
      const std::string name = mangle(member.first, id);
      const std::string type = integer_type();
      const std::string value = numeric_literal(index);
      *solver << "(declare-fun " << name << " () " << type << ")\n"
              << "(assert (= " << name << " " << value << "))\n";
      index++;
      id++;
    }
  }

  void visit_range(const Range &) final {
    // as a primitive, ranges can't contain any enum members
  }

  void visit_record(const Record &n) final {
    // define any enum members that occur in the fields of this type
    for (const Ptr<VarDecl> &field : n.fields)
      dispatch(*field->type);
  }

  void visit_typeexprid(const TypeExprID &) final {
    /* the referent of a TypeExprID will have occurred earlier in the AST and
     * already resulted in the relevant enum members being defined.
     */
  }

  void visit_scalarset(const Scalarset &) final {
    // as a primitive, scalarsets can't contain any enum members
  }
};
} // namespace

void define_enum_members(Solver &solver, const TypeExpr &type) {
  Definer definer(solver);
  definer.dispatch(type);
}

} // namespace smt
