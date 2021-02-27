#include "../../common/escape.h"
#include "generate.h"
#include "utils.h"
#include <cstddef>
#include <iostream>
#include <rumur/rumur.h>

using namespace rumur;

namespace {

class CoverAccumulator : public ConstTraversal {

private:
  std::ostream *out;
  size_t count = 0;

public:
  CoverAccumulator(std::ostream &o) : out(&o) {}

  void visit_property(const Property &n) final {
    if (n.category == Property::COVER) {
      *out << "  COVER_" << n.unique_id << " = " << count << ",\n";
      count++;
    }
  }

  size_t get_count() const { return count; }
};

class MessageGenerator : public ConstTraversal {

private:
  std::ostream *out;

public:
  MessageGenerator(std::ostream &o) : out(&o) {}

  void visit_propertyrule(const PropertyRule &n) final {
    if (n.property.category == Property::COVER) {
      if (n.name == "") {
        // No associated message. Just use the expression itself.
        *out << "  \"" << to_C_string(*n.property.expr) << "\",\n";
      } else {
        *out << "  \"" << escape(n.name) << "\",\n";
      }
    }
  }

  void visit_propertystmt(const PropertyStmt &n) final {
    if (n.property.category == Property::COVER) {
      if (n.message == "") {
        // No associated message. Just use the expression itself.
        *out << "  \"" << to_C_string(*n.property.expr) << "\",\n";
      } else {
        *out << "  \"" << escape(n.message) << "\",\n";
      }
    }
  }
};

} // namespace

void generate_cover_array(std::ostream &out, const Model &model) {
  out << "enum {\n";

  CoverAccumulator ca(out);
  ca.dispatch(model);

  out << "  /* Dummy entry in case the above generated list is empty to avoid "
         "an empty enum. */\n"
      << "  COVER_INVALID = -1,\n"
      << "};\n\n";

  out << "static const char *COVER_MESSAGES[] = {\n";
  MessageGenerator mg(out);
  mg.dispatch(model);
  out << "};\n\n";

  out << "static uintmax_t covers[" << ca.get_count() << "];\n\n";
}
