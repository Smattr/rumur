#include "generate.h"
#include <cstddef>
#include <iostream>
#include <memory>
#include <rumur/rumur.h>
#include <vector>

using namespace rumur;

namespace {

class Generator : public ConstTraversal {

private:
  std::ostream *out;

public:
  Generator(std::ostream &o) : out(&o) {}

  void visit_functioncall(const FunctionCall &n) final {
    if (n.function == nullptr)
      throw Error("function call to unresolved target " + n.name, n.loc);

    define_backing_mem(n.unique_id, n.function->return_type.get());

    for (auto &a : n.arguments)
      dispatch(*a);
  }

  virtual ~Generator() = default;

private:
  void define_backing_mem(size_t id, const TypeExpr *t) {
    if (t != nullptr && !t->is_simple())
      *out << "  uint8_t ret" << id << "[BITS_TO_BYTES(" << t->width()
           << ")];\n";
  }
};

} // namespace

void generate_allocations(std::ostream &out, const Stmt &stmt) {
  Generator g(out);
  g.dispatch(stmt);
}

void generate_allocations(std::ostream &out,
                          const std::vector<Ptr<Stmt>> &stmts) {

  for (auto &s : stmts)
    generate_allocations(out, *s);
}
