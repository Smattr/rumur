#include <cstddef>
#include <cassert>
#include "except.h"
#include "logic.h"
#include "../options.h"
#include <rumur/rumur.h>
#include <sstream>
#include <string>
#include "translate.h"
#include "typeexpr-to-smt.h"

using namespace rumur;

namespace smt {

namespace { class Translator : public ConstTypeTraversal {

 private:
  std::ostringstream out;

 public:
  Translator &operator<<(const std::string &s) {
    out << s;
    return *this;
  }

  Translator &operator<<(const TypeExpr &type) {
    dispatch(type);
    return *this;
  }

  std::string str() const {
    return out.str();
  }

  void visit_array(const Array &n) final {
    *this << "(Array " << *n.index_type << " " << *n.element_type << ")";
  }

  void visit_enum(const Enum &n) final {
    // the SMT solver already knows the type of booleans
    if (n.is_boolean()) {
      *this << "Bool";
      return;
    }

    /* we assume our caller will emit the members of this enum if they are
     * relevant to them.
     */
    *this << integer_type();
  }

  void visit_range(const Range&) final {
    /* we assume our caller will eventually set the lower and upper bound
     * constraints for this integer if it is relevant to them
     */
    *this << integer_type();
  }

  void visit_record(const Record &n) final {
    // this type will have been previously constructed using its unique
    // identifier (see define-records.cc)
    *this << mangle("", n.unique_id);
  }

  void visit_scalarset(const Scalarset&) final {
    /* A scalarset is nothing special to the SMT solver. That is, we just
     * declare it as an integer and don't expect the solver to use any symmetry
     * reasoning.
     */
    *this << integer_type();
  }

  void visit_typeexprid(const TypeExprID &n) final {
    assert(n.referent != nullptr && "unresolved TypeExprID in AST");
    *this << *n.referent->value;
  }
}; }

std::string typeexpr_to_smt(const TypeExpr &type) {
  Translator t;
  t.dispatch(type);
  return t.str();
}

}
