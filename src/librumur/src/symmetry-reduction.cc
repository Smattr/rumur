#include <iostream>
#include <rumur/Decl.h>
#include <rumur/log.h>
#include <rumur/Model.h>
#include <rumur/symmetry-reduction.h>
#include <rumur/traverse.h>
#include <rumur/TypeExpr.h>
#include <vector>

namespace rumur {

static std::vector<const TypeDecl*> find_scalarsets(const Model &m) {
  std::vector<const TypeDecl*> ss;
  for (const Decl *d : m.decls) {
    if (auto t = dynamic_cast<const TypeDecl*>(d)) {
      if (dynamic_cast<const Scalarset*>(t->value) != nullptr)
        ss.push_back(t);
    }
  }
  return ss;
}

static void generate_pivot_data(const TypeDecl &t, std::ostream &out) {

  auto s = dynamic_cast<const Scalarset&>(*t.value);
  int64_t b = s.bound->constant_fold();

  out
    /* An array indicating how to sort the given scalarset. For example, if we
     * have a scalarset(4), this array may end up as { 3, 1, 0, 2 }. This would
     * indicate any other scalarset state data encountered should map 0 to 3, 1
     * to itself, 2 to 0, and 3 to 2.
     */
    << "  size_t schedule_" << t.name << "[" << b << "] "
      << "__attribute__((unused));\n"

    /* A flag indicating whether we have yet found a "pivot," an array indexed
     * by this scalarset whose contents can be sorted to derive a "schedule"
     * (above). The contents of the schedule should not be considered valid
     * until this flag is true. Conversely if we encounter a sorting candidate
     * and this flag is false, we can consider the candidate the pivot and
     * write the schedule.
     */
    << "  bool pivoted_" << t.name << " __attribute__((unused)) = false;\n";
}

void generate_canonicalise(const Model &m, std::ostream &out) {

  // Write the function header
  out
    << "static void state_canonicalise(struct state *s "
      << "__attribute__((unused))) {\n";

  // Define the schedule and pivot
  std::vector<const TypeDecl*> ss = find_scalarsets(m);
  *log.info << "symmetry reduction: " << ss.size() << " eligible scalarset "
    "types\n";
  for (const TypeDecl *t : ss)
    generate_pivot_data(*t, out);

  out << "}";
}

}
