#include <cstddef>
#include <iostream>
#include <rumur/rumur.h>

using namespace rumur;

namespace {

class CGenerator : public ConstTraversal {

 private:
  std::ostream &out;

 public:
  explicit CGenerator(std::ostream &out_): out(out_) { }

  void visit_model(const Model&) final {
    out << "TODO\n";
  }

  virtual ~CGenerator() = default;
};

}

void generate_c(const Node &n, std::ostream &out) {
  CGenerator gen(out);
  gen.dispatch(n);
}
