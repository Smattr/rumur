#include "generate.h"
#include <iostream>
#include <rumur/rumur.h>
#include "utils.h"

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

class MessageGenerator : public rumur::ConstTraversal {

 private:
  std::ostream *out;

 public:
  MessageGenerator(std::ostream &o): out(&o) { }

  void visit_propertyrule(const rumur::PropertyRule &n) final {
    if (n.property.category == rumur::Property::COVER) {
      if (n.name == "") {
        // No associated message. Just use the expression itself.
        *out << "  \"" << to_C_string(*n.property.expr) << "\",\n";
      } else {
        *out << "  \"" << escape(n.name) << "\",\n";
      }
    }
  }

  void visit_propertystmt(const rumur::PropertyStmt &n) final {
    if (n.property.category == rumur::Property::COVER) {
      if (n.message == "") {
        // No associated message. Just use the expression itself.
        *out << "  \"" << to_C_string(*n.property.expr) << "\",\n";
      } else {
        *out << "  \"" << escape(n.message) << "\",\n";
      }
    }
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
    << "};\n\n";

  out << "static const char *COVER_MESSAGES[] = {\n";
  MessageGenerator mg(out);
  mg.dispatch(model);
  out << "};\n\n";

  out << "static atomic_uintmax_t covers[" << ca.get_count() << "];\n\n";
}
