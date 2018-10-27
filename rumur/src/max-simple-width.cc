#include <gmpxx.h>
#include <memory>
#include <rumur/rumur.h>

// An AST traversal that learns the maximum simple type width.
class Traversal : public rumur::ConstTypeTraversal {

 public:
  mpz_class max = 0;

  /* Nothing required for complex types, but we do need to descend into their
   * children.
   */
  void visit(const rumur::Array &n) {
    dispatch(*n.index_type);
    dispatch(*n.element_type);
  }

  void visit(const rumur::Enum &n) {
    mpz_class w = n.width();
    if (w > max)
      max = w;
  }

  void visit(const rumur::Range &n) {
    mpz_class w = n.width();
    if (w > max)
      max = w;
  }

  void visit(const rumur::Record &n) {
    for (const std::shared_ptr<rumur::VarDecl> &f : n.fields)
      dispatch(*f);
  }

  void visit(const rumur::Scalarset &n) {
    mpz_class w = n.width();
    if (w > max)
      max = w;
  }

  /* We don't need to do anything for type identifiers, because we know we will
   * have seen the underlying type somewhere else already.
   */
  void visit(const rumur::TypeExprID&) { }
};

mpz_class max_simple_width(const rumur::Model &m) {
  Traversal t;
  t.dispatch(m);
  return t.max;
}
