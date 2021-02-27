#include "symmetry-reduction.h"
#include "../../common/isa.h"
#include "options.h"
#include "utils.h"
#include <cassert>
#include <cstddef>
#include <gmpxx.h>
#include <iostream>
#include <memory>
#include <rumur/rumur.h>
#include <utility>
#include <vector>

using namespace rumur;

std::vector<const TypeDecl *> get_scalarsets(const Model &m) {
  std::vector<const TypeDecl *> ss;
  for (const Ptr<Node> &c : m.children) {
    if (auto t = dynamic_cast<const TypeDecl *>(c.get())) {
      if (isa<Scalarset>(t->value))
        ss.push_back(t);
    }
  }
  return ss;
}

// XXX: GMP 6.2.0 contains a function for this, but we want to retain
// compatibility with older GMP
static mpz_class factorial(mpz_class v) {

  assert(v >= 0);

  mpz_class r = 1;

  while (v != 0) {
    r *= v;
    --v;
  }

  return r;
}

mpz_class get_schedule_width(const TypeDecl &t) {

  auto s = dynamic_cast<const Scalarset *>(t.value.get());
  assert(s != nullptr && "non-scalarset passed to get_schedule_width()");

  // how many unique non-undefined values of this scalarset are there?
  mpz_class bound = s->count() - 1;
  assert(bound > 0);

  // how many permutations of this many values are there?
  mpz_class perm = factorial(bound);
  assert(perm > 0);

  // and how many bits do we need to store this many values?
  return bit_width(perm);
}

// Generate application of a swap of two state components
static void generate_apply_swap(std::ostream &out, const std::string &offset_a,
                                const std::string &offset_b,
                                const TypeExpr &type, size_t depth = 0) {

  const Ptr<TypeExpr> t = type.resolve();

  const std::string indent = std::string((depth + 1) * 2, ' ');

  if (t->is_simple()) {

    out << indent << "if (" << offset_a << " != " << offset_b << ") {\n"
        << indent << "  raw_value_t a = handle_read_raw(s, state_handle(s, "
        << offset_a << ", " << t->width() << "ull));\n"
        << indent << "  raw_value_t b = handle_read_raw(s, state_handle(s, "
        << offset_b << ", " << t->width() << "ull));\n"
        << indent << "  handle_write_raw(s, state_handle(s, " << offset_b
        << ", " << t->width() << "ull), a);\n"
        << indent << "  handle_write_raw(s, state_handle(s, " << offset_a
        << ", " << t->width() << "ull), b);\n"
        << indent << "}\n";
    return;
  }

  if (auto a = dynamic_cast<const Array *>(t.get())) {
    const std::string var = "i" + std::to_string(depth);
    mpz_class ic = a->index_type->count() - 1;
    const std::string len = "((size_t)" + ic.get_str() + "ull)";
    const std::string width =
        "((size_t)" + a->element_type->width().get_str() + "ull)";

    out << indent << "for (size_t " << var << " = 0; " << var << " < " << len
        << "; " << var << "++) {\n";

    const std::string off_a = offset_a + " + " + var + " * " + width;
    const std::string off_b = offset_b + " + " + var + " * " + width;

    generate_apply_swap(out, off_a, off_b, *a->element_type, depth + 1);

    out << indent << "}\n";
    return;
  }

  if (auto r = dynamic_cast<const Record *>(t.get())) {
    std::string off_a = offset_a;
    std::string off_b = offset_b;

    for (const Ptr<VarDecl> &f : r->fields) {
      generate_apply_swap(out, off_a, off_b, *f->type, depth);

      off_a += " + ((size_t)" + f->width().get_str() + "ull)";
      off_b += " + ((size_t)" + f->width().get_str() + "ull)";
    }
    return;
  }

  assert(!"missed case in generate_apply_swap");
}

static void generate_swap_chunk(std::ostream &out, const TypeExpr &t,
                                const std::string &offset,
                                const TypeDecl &pivot, size_t depth = 0) {

  const std::string indent((depth + 1) * 2, ' ');

  if (t.is_simple()) {

    if (auto s = dynamic_cast<const TypeExprID *>(&t)) {
      if (s->name == pivot.name) {
        /* This state component has the same type as the pivot. If its value is
         * one of the pair we are swapping, we need to change it to the other.
         */

        const std::string w = "((size_t)" + t.width().get_str() + "ull)";
        const std::string h = "state_handle(s, " + offset + ", " + w + ")";

        out << indent << "if (x != y) {\n"
            << indent << "  raw_value_t v = handle_read_raw(s, " << h << ");\n"
            << indent << "  if (v != 0) {\n"
            << indent << "    if (v - 1 == (raw_value_t)x) {\n"
            << indent << "      handle_write_raw(s, " << h << ", y + 1);\n"
            << indent << "    } else if (v - 1 == (raw_value_t)y) {\n"
            << indent << "      handle_write_raw(s, " << h << ", x + 1);\n"
            << indent << "    }\n"
            << indent << "  }\n"
            << indent << "}\n";
      }
    }

    // A component of any other simple type is irrelevant.

    return;
  }

  const Ptr<TypeExpr> type = t.resolve();

  if (auto a = dynamic_cast<const Array *>(type.get())) {

    const std::string w =
        "((size_t)" + a->element_type->width().get_str() + "ull)";

    // If this array is indexed by our pivot type, swap the relevant elements
    auto s = dynamic_cast<const TypeExprID *>(a->index_type.get());
    if (s != nullptr && s->name == pivot.name) {

      const std::string off_x = offset + " + x * " + w;
      const std::string off_y = offset + " + y * " + w;

      generate_apply_swap(out, off_x, off_y, *a->element_type, depth);
    }

    // Descend into its element to allow further swapping

    const std::string i = "i" + std::to_string(depth);
    mpz_class ic = a->index_type->count() - 1;
    const std::string len = "((size_t)" + ic.get_str() + "ull)";

    out << indent << "for (size_t " << i << " = 0; " << i << " < " << len
        << "; " << i << "++) {\n";

    const std::string off = offset + " + " + i + " * " + w;

    generate_swap_chunk(out, *a->element_type, off, pivot, depth + 1);

    out << indent << "}\n";

    return;
  }

  if (auto r = dynamic_cast<const Record *>(type.get())) {

    std::string off = offset;

    for (const Ptr<VarDecl> &f : r->fields) {
      generate_swap_chunk(out, *f->type, off, pivot, depth);

      off += " + ((size_t)" + f->width().get_str() + "ull)";
    }
    return;
  }

  assert(!"missed case in generate_swap_chunk");
}

static void generate_swap(const Model &m, std::ostream &out,
                          const TypeDecl &pivot) {

  out << "static void swap_" << pivot.name << "("
      << "struct state *s __attribute__((unused)), "
      << "size_t x __attribute__((unused)), "
      << "size_t y __attribute__((unused))) {\n";

  for (const Ptr<Node> &c : m.children) {
    if (auto v = dynamic_cast<const VarDecl *>(c.get())) {
      std::string offset = "((size_t)" + v->offset.get_str() + "ull)";
      generate_swap_chunk(out, *v->type, offset, pivot);
    }
  }

  out << "}\n\n";
}

static void generate_schedule_reader(std::ostream &out, const TypeDecl &pivot,
                                     const mpz_class &offset,
                                     const mpz_class &width) {

  out << "static size_t schedule_read_" << pivot.name
      << "(const struct state *NONNULL s) {\n"
      << "  assert(s != NULL);\n"
      << "  return state_schedule_get(s, " << offset.get_str() << "ul, "
      << width.get_str() << "ul);\n"
      << "}\n";
}

static void generate_schedule_writer(std::ostream &out, const TypeDecl &pivot,
                                     const mpz_class &offset,
                                     const mpz_class &width) {

  out << "static void schedule_write_" << pivot.name
      << "(struct state *NONNULL s, size_t schedule_index) {\n"
      << "  assert(s != NULL);\n"
      << "  state_schedule_set(s, " << offset.get_str() << "ul, "
      << width.get_str() << "ul, schedule_index);\n"
      << "}\n";
}

static void generate_loop_header(const TypeDecl &scalarset, size_t index,
                                 size_t level, std::ostream &out) {

  const std::string indent(level * 2, ' ');

  const Ptr<TypeExpr> type = scalarset.value->resolve();
  auto s = dynamic_cast<const Scalarset *>(type.get());
  assert(s != nullptr);

  const std::string bound =
      "((size_t)" + s->bound->constant_fold().get_str() + "ull)";
  const std::string i = "i" + std::to_string(index);

  out << indent << "if (state_cmp(&candidate, s) < 0) {\n"
      << indent << "  /* Found a more canonical representation. */\n"
      << indent << "  memcpy(s, &candidate, sizeof(*s));\n"
      << indent << "}\n\n"

      << indent << "{\n"
      << indent << "  size_t stack_" << scalarset.name << "[" << bound
      << "] = { 0 };\n\n"

      << indent << "  size_t schedule_" << scalarset.name << "[" << bound
      << "] = { 0 };\n"
      << indent << "  if (USE_SCALARSET_SCHEDULES) {\n"
      << indent << "    size_t stack[" << bound << "];\n"
      << indent << "    size_t index = schedule_read_" << scalarset.name
      << "(&candidate);\n"
      << indent << "    index_to_permutation(index, schedule_" << scalarset.name
      << ", stack, " << bound << ");\n"
      << indent << "  }\n\n"

      << indent << "  for (size_t " << i << " = 0; " << i << " < " << bound
      << "; ) {\n"
      << indent << "    if (stack_" << scalarset.name << "[" << i << "] < " << i
      << ") {\n"
      << indent << "      if (" << i << " % 2 == 0) {\n"
      << indent << "        swap_" << scalarset.name << "(&candidate, 0, " << i
      << ");\n"
      << indent << "        size_t tmp = schedule_" << scalarset.name
      << "[0];\n"
      << indent << "        schedule_" << scalarset.name << "[0] = schedule_"
      << scalarset.name << "[" << i << "];\n"
      << indent << "        schedule_" << scalarset.name << "[" << i
      << "] = tmp;\n"
      << indent << "      } else {\n"
      << indent << "        swap_" << scalarset.name << "(&candidate, stack_"
      << scalarset.name << "[" << i << "], " << i << ");\n"
      << indent << "        size_t tmp = schedule_" << scalarset.name
      << "[stack_" << scalarset.name << "[" << i << "]];\n"
      << indent << "        schedule_" << scalarset.name << "[stack_"
      << scalarset.name << "[" << i << "]] = schedule_" << scalarset.name << "["
      << i << "];\n"
      << indent << "        schedule_" << scalarset.name << "[" << i
      << "] = tmp;\n"
      << indent << "      }\n"
      << indent
      << "      /* save selected schedule to map this back for later more\n"
      << indent << "       * comprehensible counterexample traces\n"
      << indent << "       */\n"
      << indent << "      if (USE_SCALARSET_SCHEDULES) {\n"
      << indent << "        size_t stack[" << bound << "];\n"
      << indent << "        size_t working[" << bound << "];\n"
      << indent << "        size_t index = permutation_to_index(schedule_"
      << scalarset.name << ", stack, working, " << bound << ");\n"
      << indent << "        schedule_write_" << scalarset.name
      << "(&candidate, index);\n"
      << indent << "      }\n";
}

static void generate_loop_footer(const TypeDecl &scalarset, size_t index,
                                 size_t level, std::ostream &out) {

  const std::string indent(level * 2, ' ');

  assert(isa<Scalarset>(scalarset.value->resolve()));

  const std::string i = "i" + std::to_string(index);

  out << indent << "      stack_" << scalarset.name << "[" << i << "]++;\n"
      << indent << "      " << i << " = 0;\n"
      << indent << "    } else {\n"
      << indent << "      stack_" << scalarset.name << "[" << i << "] = 0;\n"
      << indent << "      " << i << "++;\n"
      << indent << "    }\n"
      << indent << "  }\n"
      << indent << "}\n";
}

static void generate_loop(const std::vector<const TypeDecl *> &scalarsets,
                          size_t index, size_t level, std::ostream &out) {

  if (index < scalarsets.size() - 1)
    generate_loop(scalarsets, index + 1, level, out);

  generate_loop_header(*scalarsets[index], index, level, out);

  if (index < scalarsets.size() - 1) {
    generate_loop(scalarsets, index + 1, level + 3, out);
  } else {
    const std::string indent((level + 3) * 2, ' ');

    out << indent << "if (state_cmp(&candidate, s) < 0) {\n"
        << indent << "  /* Found a more canonical representation. */\n"
        << indent << "  memcpy(s, &candidate, sizeof(*s));\n"
        << indent << "}\n\n";
  }

  generate_loop_footer(*scalarsets[index], index, level, out);
}

static void generate_canonicalise_exhaustive(
    const std::vector<const TypeDecl *> &scalarsets, std::ostream &out) {

  // Write the function prelude
  out << "static void state_canonicalise_exhaustive(struct state *s "
         "__attribute__((unused))) {\n"
      << "\n"
      << "  assert(s != NULL && \"attempt to canonicalise NULL state\");\n"
      << "\n";

  if (!scalarsets.empty()) {
    out << "  /* A state to store the current permutation we are considering. "
           "*/\n"
        << "  static _Thread_local struct state candidate;\n"
        << "  memcpy(&candidate, s, sizeof(candidate));\n"
        << "\n";

    generate_loop(scalarsets, 0, 1, out);
  }

  // Write the function coda
  out << "}\n\n";
}

static bool is_pivot(const TypeDecl &pivot, const TypeExpr *t) {

  if (t == nullptr)
    return false;

  auto s = dynamic_cast<const TypeExprID *>(t);
  if (s == nullptr)
    return false;

  return pivot.name == s->name;
}

// Generate application of a comparison of two state components
static void generate_apply_compare(std::ostream &out, const TypeExpr &type,
                                   const std::string &offset_a,
                                   const std::string &offset_b,
                                   const TypeDecl &pivot, size_t depth = 0,
                                   bool used_pivot = false) {

  const Ptr<TypeExpr> t = type.resolve();

  const std::string indent((depth + 1) * 2, ' ');

  if (t->is_simple()) {

    /* Emit a comparison of this state component, unless it's scoped by the same
     * scalarset as ourselves. If we're already doing a comparison based on this
     * scalarset, we don't want to reuse it because this value itself will
     * change if/when the parent (array) component is reshuffled.
     */
    if (!used_pivot || !is_pivot(pivot, &type)) {

      out << indent << "if (" << offset_a << " != " << offset_b << ") {\n"
          << indent << "  raw_value_t a = handle_read_raw(s, state_handle(s, "
          << offset_a << ", " << t->width() << "ull));\n"
          << indent << "  raw_value_t b = handle_read_raw(s, state_handle(s, "
          << offset_b << ", " << t->width() << "ull));\n"
          << indent << "  if (a < b) {\n"
          << indent << "    return -1;\n"
          << indent << "  } else if (a > b) {\n"
          << indent << "    return 1;\n"
          << indent << "  }\n"
          << indent << "}\n";
    }

    return;
  }

  if (auto a = dynamic_cast<const Array *>(t.get())) {

    if (!used_pivot || !is_pivot(pivot, a->index_type.get())) {

      used_pivot |= is_pivot(pivot, a->index_type.get());

      const std::string var = "i" + std::to_string(depth);
      mpz_class ic = a->index_type->count() - 1;
      const std::string len = "((size_t)" + ic.get_str() + "ull)";
      const std::string width =
          "((size_t)" + a->element_type->width().get_str() + "ull)";

      out << indent << "for (size_t " << var << " = 0; " << var << " < " << len
          << "; " << var << "++) {\n";

      const std::string off_a = offset_a + " + " + var + " * " + width;
      const std::string off_b = offset_b + " + " + var + " * " + width;

      generate_apply_compare(out, *a->element_type, off_a, off_b, pivot,
                             depth + 1, used_pivot);

      out << indent << "}\n";
    }

    return;
  }

  if (auto r = dynamic_cast<const Record *>(t.get())) {
    std::string off_a = offset_a;
    std::string off_b = offset_b;

    for (const Ptr<VarDecl> &f : r->fields) {
      generate_apply_compare(out, *f->type, off_a, off_b, pivot, depth,
                             used_pivot);

      off_a += " + ((size_t)" + f->width().get_str() + "ull)";
      off_b += " + ((size_t)" + f->width().get_str() + "ull)";
    }
    return;
  }

  assert(!"missed case in generate_apply_compare");
}

// Generate part of a memcmp-style comparator
static void generate_compare_chunk(std::ostream &out, const TypeExpr &t,
                                   const std::string offset,
                                   const TypeDecl &pivot, size_t depth = 0,
                                   bool used_pivot = false) {

  const std::string indent((depth + 1) * 2, ' ');

  if (t.is_simple()) {
    /* If this state component has the same type as the pivot, we need to see if
     * it matches either of the operands. Here, we are basically looking to see
     * which (if either) of the scalarset elements appears *first* in the state.
     */
    if (is_pivot(pivot, &t) && !used_pivot) {

      const std::string width = "((size_t)" + t.width().get_str() + "ull)";
      out

          /* Open a scope so we don't need to think about redeclaring/shadowing
           * 'v'.
           */
          << indent << "{\n"

          << indent << "  raw_value_t v = handle_read_raw(s, state_handle(s, "
          << offset << ", " << width << "));\n"

          << indent << "  if (v != 0) { /* ignored 'undefined' */\n"
          << indent << "    if (v - 1 == (raw_value_t)x) {\n"
          << indent << "      return -1;\n"
          << indent << "    } else if (v - 1 == (raw_value_t)y) {\n"
          << indent << "      return 1;\n"
          << indent << "    }\n"
          << indent
          << "  }\n"

          // Close scope
          << indent << "}\n";
    }

    // Nothing required for any other simply type.

    return;
  }

  const Ptr<TypeExpr> type = t.resolve();

  if (auto a = dynamic_cast<const Array *>(type.get())) {

    // The bit size of each array element as a C code string
    const std::string width =
        "((size_t)" + a->element_type->width().get_str() + "ull)";

    /* If this array is indexed by the pivot type, first compare the relevant
     * elements. Note, we'll only end up descending if the two elements happen
     * to be equal.
     */
    if (is_pivot(pivot, a->index_type.get()) && !used_pivot) {

      const std::string off_x = offset + " + x * " + width;
      const std::string off_y = offset + " + y * " + width;

      generate_apply_compare(out, *a->element_type, off_x, off_y, pivot, depth,
                             true);

      used_pivot = true;
    }

    // Descend into its elements to allow further comparison

    // The number of elements in this array as a C code string
    mpz_class ic = a->index_type->count() - 1;
    const std::string ub = "((size_t)" + ic.get_str() + "ull)";

    // Generate a loop to iterate over all the elements
    const std::string var = "i" + std::to_string(depth);
    out << indent << "for (size_t " << var << " = 0; " << var << " < " << ub
        << "; " << var << "++) {\n";

    // Generate code to compare each element
    const std::string off = offset + " + " + var + " * " + width;
    generate_compare_chunk(out, *a->element_type, off, pivot, depth + 1,
                           used_pivot);

    // Close the loop
    out << indent << "}\n";

    return;
  }

  if (auto r = dynamic_cast<const Record *>(type.get())) {

    std::string off = offset;

    for (const Ptr<VarDecl> &f : r->fields) {

      // Generate code to compare this field
      generate_compare_chunk(out, *f->type, off, pivot, depth, used_pivot);

      // Jump over this field to get the offset of the next field
      const std::string width = "((size_t)" + f->width().get_str() + "ull)";
      off += " + " + width;
    }

    return;
  }

  assert(!"missed case in generate_compare_chunk");
}

/* Generate a memcmp-style comparator for a given scalarset with respect to the
 * state.
 */
static void generate_compare(std::ostream &out, const TypeDecl &pivot,
                             const Model &m) {

  out << "static int compare_" << pivot.name << "(const struct state *s, "
      << "size_t x, size_t y) {\n"
      << "\n"
      << "  if (x == y) {\n"
      << "    return 0;\n"
      << "  }\n"
      << "\n";

  for (const Ptr<Node> &c : m.children) {
    if (auto v = dynamic_cast<const VarDecl *>(c.get())) {
      const std::string offset = "((size_t)" + v->offset.get_str() + "ull)";
      generate_compare_chunk(out, *v->type, offset, pivot);
    }
  }

  out
      // Fall through case where all components were equal
      << "  return 0;\n"
      << "}\n";
}

static void generate_sort(std::ostream &out, const TypeDecl &pivot) {

  assert(isa<Scalarset>(pivot.value->resolve()));

  out << "static void sort_" << pivot.name << "(struct state *s, "
      << "size_t *schedule, size_t lower, size_t upper) {\n"
      << "\n"
      << "  /* If we have nothing to sort, bail out. */\n"
      << "  if (lower >= upper) {\n"
      << "    return;\n"
      << "  }\n"
      << "\n"
      << "  /* Use Hoare's partitioning algorithm to apply quicksort. */\n"
      << "  size_t pivot = lower;\n" // <- this is "pivot" in the quicksort
                                     // sense
      << "  size_t i = lower - 1;\n"
      << "  size_t j = upper + 1;\n"
      << "\n"
      << "  for (;;) {\n"
      << "\n"
      << "    do {\n"
      << "      i++;\n"
      << "      assert(i >= lower && i <= upper && \"out of bounds access in "
      << "sort_" << pivot.name << "()\");\n"
      << "    } while (compare_" << pivot.name << "(s, i, pivot) < 0);\n"
      << "\n"
      << "    do {\n"
      << "      j--;\n"
      << "      assert(j >= lower && j <= upper && \"out of bounds access in "
      << "sort_" << pivot.name << "()\");\n"
      << "    } while (compare_" << pivot.name << "(s, j, pivot) > 0);\n"
      << "\n"
      << "    if (i >= j) {\n"
      << "      break;\n"
      << "    }\n"
      << "\n"
      << "    /* Swap elements i and j. */\n"
      << "    swap_" << pivot.name << "(s, i, j);\n"
      << "    {\n"
      << "      size_t tmp = schedule[i];\n"
      << "      schedule[i] = schedule[j]; \n"
      << "      schedule[j] = tmp;\n"
      << "    }\n"
      << "    if (i == pivot) {\n"
      << "      pivot = j;\n"
      << "    } else if (j == pivot) {\n"
      << "      pivot = i;\n"
      << "    }\n"
      << "  }\n"
      << "\n"
      << "  sort_" << pivot.name << "(s, schedule, lower, j);\n"
      << "  sort_" << pivot.name << "(s, schedule, j + 1, upper);\n"
      << "}\n";
}

static void
generate_canonicalise_heuristic(const Model &m,
                                const std::vector<const TypeDecl *> &scalarsets,
                                std::ostream &out) {

  for (const TypeDecl *t : scalarsets) {
    generate_compare(out, *t, m);
    generate_sort(out, *t);
  }

  out << "static void state_canonicalise_heuristic(struct state *s "
         "__attribute__((unused))) {\n"
      << "\n"
      << "  assert(s != NULL && \"attempt to canonicalise NULL state\");\n"
      << "\n";

  for (const TypeDecl *t : scalarsets) {

    const Ptr<TypeExpr> type = t->value->resolve();
    auto s = dynamic_cast<const Scalarset *>(type.get());
    assert(s != nullptr);

    mpz_class bound = s->count() - 1;

    out << "  {\n"
        << "    size_t schedule[(size_t)" << bound.get_str() << "ull];\n"
        << "    for (size_t i = 0; i < sizeof(schedule) / sizeof(schedule[0]); "
        << "++i) {\n"
        << "      schedule[i] = i;\n"
        << "    }\n"
        << "    if (USE_SCALARSET_SCHEDULES) {\n"
        << "      size_t index = schedule_read_" << t->name << "(s);\n"
        << "      size_t stack[(size_t)" << bound.get_str() << "ull];\n"
        << "      index_to_permutation(index, schedule, stack, "
        << bound.get_str() << "ull);\n"
        << "    }\n"
        << "    sort_" << t->name << "(s, schedule, 0, ((size_t)"
        << bound.get_str() << "ull) - 1);\n"
        << "    /* save selected schedule to map this back for later more\n"
        << "     * comprehensible counterexample traces\n"
        << "     */\n"
        << "    if (USE_SCALARSET_SCHEDULES) {\n"
        << "      size_t stack[(size_t)" << bound.get_str() << "ull];\n"
        << "      size_t working[(size_t)" << bound.get_str() << "ull];\n"
        << "      size_t index = permutation_to_index(schedule, stack, "
           "working, "
        << bound.get_str() << "ull);\n"
        << "      schedule_write_" << t->name << "(s, index);\n"
        << "    }\n"
        << "  }\n";
  }

  out << "}\n\n";
}

void generate_canonicalise(const Model &m, std::ostream &out) {

  // Find types eligible for use in canonicalisation
  const std::vector<const TypeDecl *> scalarsets = get_scalarsets(m);

  // Generate functions to swap state elements with respect to each scalarset
  for (const TypeDecl *t : scalarsets)
    generate_swap(m, out, *t);

  // generate functions to read and write schedule (which permutation was
  // selected) data
  {
    mpz_class offset = 0;
    for (const TypeDecl *t : scalarsets) {
      const mpz_class width = get_schedule_width(*t);
      generate_schedule_reader(out, *t, offset, width);
      generate_schedule_writer(out, *t, offset, width);
      offset += width;
    }
  }

  generate_canonicalise_exhaustive(scalarsets, out);

  generate_canonicalise_heuristic(m, scalarsets, out);
}
