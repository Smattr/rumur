#include "generate.h"
#include <iostream>
#include <rumur/rumur.h>

namespace {

class CoverAccumulator : public rumur::ConstTraversal {

 private:
  std::ostream *out;
  size_t count = 0;

 public:
  CoverAccumulator(std::ostream &o): out(&o) { }

  void visit_property(const rumur::Property &n) final {
    if (n.category == rumur::Property::COVER) {
      *out << "  COVER_" << n.unique_id << " = " << count << ",\n";
      count++;
    }
  }

  size_t get_count() const {
    return count;
  }
};

}

void generate_cover_array(std::ostream &out, const rumur::Model &model) {
  out << "enum {\n";

  CoverAccumulator ca(out);
  ca.dispatch(model);

  out
    << "  /* Dummy entry in case the above generated list is empty to avoid an empty enum. */\n"
    << "  COVER_INVALID = -1,\n"
    << "};\n\n"
    << "static atomic_uintmax_t covers[" << ca.get_count() << "];\n\n";
}
