#include <cstddef>
#include <gmpxx.h>
#include "max-simple-width.h"
#include <memory>
#include <rumur/rumur.h>

using namespace rumur;

// An AST traversal that learns the maximum simple type width.
namespace { class Measurer : public ConstTraversal {

 public:
  mpz_class max = 0;

  /* Nothing required for complex types, but we do need to descend into their
   * children.
   */
  void visit_array(const Array &n) final {
    dispatch(*n.index_type);
    dispatch(*n.element_type);
  }

  void visit_enum(const Enum &n) final {
    mpz_class w = n.width();
    if (w > max)
      max = w;
  }

  void visit_range(const Range &n) final {
    mpz_class w = n.width();
    if (w > max)
      max = w;
  }

  void visit_record(const Record &n) final {
    for (auto &f : n.fields)
      dispatch(*f);
  }

  void visit_scalarset(const Scalarset &n) final {
    mpz_class w = n.width();
    if (w > max)
      max = w;
  }

  void visit_typeexprid(const TypeExprID &n) final {
    if (n.is_simple()) {
      mpz_class w = n.width();
      if (w > max)
        max = w;
    }
    /* We don't need to descend into any children, because the referent will
     * either (a) already have been encountered or (b) be a built-in like
     * 'boolean' that is a simple type.
     */
  }
}; }

mpz_class max_simple_width(const Model &m) {
  Measurer t;
  t.dispatch(m);
  return t.max;
}
