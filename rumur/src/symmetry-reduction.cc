#include <cassert>
#include <cstddef>
#include <gmpxx.h>
#include <iostream>
#include <memory>
#include "options.h"
#include <rumur/rumur.h>
#include "symmetry-reduction.h"
#include <utility>
#include "utils.h"
#include <vector>

using namespace rumur;

// Find all the named scalarset declarations in a model.
static std::vector<const TypeDecl*> get_scalarsets(const Model &m) {
  std::vector<const TypeDecl*> ss;
  for (const std::shared_ptr<Decl> &d : m.decls) {
    if (auto t = dynamic_cast<const TypeDecl*>(d.get())) {
      if (isa<Scalarset>(t->value))
        ss.push_back(t);
    }
  }
  return ss;
}

// Generate application of a swap of two state components
static void generate_apply_swap(std::ostream &out, const std::string &offset_a,
  const std::string &offset_b, const TypeExpr &type, size_t depth = 0) {

  const TypeExpr *t = type.resolve();

  const std::string indent = std::string((depth + 1) * 2, ' ');

  if (t->is_simple()) {

    out
      << indent << "if (" << offset_a << " != " << offset_b << ") {\n"
      << indent << "  value_t a = handle_read_raw(state_handle(s, " << offset_a
        << ", SIZE_C(" << t->width() << ")));\n"
      << indent << "  value_t b = handle_read_raw(state_handle(s, " << offset_b
        << ", SIZE_C(" << t->width() << ")));\n"
      << indent << "  handle_write_raw(state_handle(s, " << offset_b
        << ", SIZE_C(" << t->width() << ")), a);\n"
      << indent << "  handle_write_raw(state_handle(s, " << offset_a
        << ", SIZE_C(" << t->width() << ")), b);\n"
      << indent << "}\n";
    return;
  }

  if (auto a = dynamic_cast<const Array*>(t)) {
    const std::string var = "i" + std::to_string(depth);
    mpz_class ic = a->index_type->count() - 1;
    const std::string len = "SIZE_C(" + ic.get_str() + ")";
    const std::string width = "SIZE_C("
      + a->element_type->width().get_str() + ")";

    out << indent << "for (size_t " << var << " = 0; " << var << " < " << len
      << "; " << var << "++) {\n";

    const std::string off_a = offset_a + " + " + var + " * " + width;
    const std::string off_b = offset_b + " + " + var + " * " + width;

    generate_apply_swap(out, off_a, off_b, *a->element_type, depth + 1);

    out << indent << "}\n";
    return;
  }

  if (auto r = dynamic_cast<const Record*>(t)) {
    std::string off_a = offset_a;
    std::string off_b = offset_b;

    for (const std::shared_ptr<VarDecl> &f : r->fields) {
      generate_apply_swap(out, off_a, off_b, *f->type, depth);

      off_a += " + SIZE_C(" + f->width().get_str() + ")";
      off_b += " + SIZE_C(" + f->width().get_str() + ")";
    }
    return;
  }

  assert(!"missed case in generate_apply_swap");
}

static void generate_swap_chunk(std::ostream &out, const TypeExpr &t,
    const std::string &offset, const TypeDecl &pivot, size_t depth = 0) {

  const std::string indent((depth + 1) * 2, ' ');

  if (t.is_simple()) {

    if (auto s = dynamic_cast<const TypeExprID*>(&t)) {
      if (s->name == pivot.name) {
        /* This state component has the same type as the pivot. If its value is
         * one of the pair we are swapping, we need to change it to the other.
         */

        const std::string w = "SIZE_C(" + t.width().get_str() + ")";
        const std::string h = "state_handle(s, " + offset + ", " + w + ")";

        out
          << indent << "if (x != y) {\n"
          << indent << "  value_t v = handle_read_raw(" << h << ");\n"
          << indent << "  if (v != 0) {\n"
          << indent << "    if (v - 1 == (value_t)x) {\n"
          << indent << "      handle_write_raw(" << h << ", y + 1);\n"
          << indent << "    } else if (v - 1 == (value_t)y) {\n"
          << indent << "      handle_write_raw(" << h << ", x + 1);\n"
          << indent << "    }\n"
          << indent << "  }\n"
          << indent << "}\n";
      }
    }

    // A component of any other simple type is irrelevant.

    return;
  }

  if (auto a = dynamic_cast<const Array*>(t.resolve())) {

    const std::string w = "SIZE_C(" + a->element_type->width().get_str() + ")";

    // If this array is indexed by our pivot type, swap the relevant elements
    auto s = dynamic_cast<const TypeExprID*>(a->index_type.get());
    if (s != nullptr && s->name == pivot.name) {

      const std::string off_x = offset + " + x * " + w;
      const std::string off_y = offset + " + y * " + w;

      generate_apply_swap(out, off_x, off_y, *a->element_type, depth);
    }

    // Descend into its element to allow further swapping

    const std::string i = "i" + std::to_string(depth);
    mpz_class ic = a->index_type->count() - 1;
    const std::string len = "SIZE_C(" + ic.get_str() + ")";

    out
      << indent << "for (size_t " << i << " = 0; " << i << " < " << len << "; "
        << i << "++) {\n";

    const std::string off = offset + " + " + i + " * " + w;

    generate_swap_chunk(out, *a->element_type, off, pivot, depth + 1);

    out << indent << "}\n";

    return;
  }

  if (auto r = dynamic_cast<const Record*>(t.resolve())) {

    std::string off = offset;

    for (const std::shared_ptr<VarDecl> &f : r->fields) {
      generate_swap_chunk(out, *f->type, off, pivot, depth);

      off += " + SIZE_C(" + f->width().get_str() + ")";
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

  for (const std::shared_ptr<Decl> &d : m.decls) {
    if (auto v = dynamic_cast<const VarDecl*>(d.get())) {
      std::string offset = "SIZE_C(" + v->offset.get_str() + ")";
      generate_swap_chunk(out, *v->type, offset, pivot);
    }
  }

  out << "}\n\n";
}

static void generate_loop_header(const TypeDecl &scalarset, size_t index,
    size_t level, std::ostream &out) {

  const std::string indent(level * 2, ' ');

  auto s = dynamic_cast<const Scalarset*>(scalarset.value->resolve());
  assert(s != nullptr);

  const std::string  bound = "SIZE_C(" + s->bound->constant_fold().get_str() + ")";
  const std::string i = "i" + std::to_string(index);

  out
    << indent << "if (state_cmp(&candidate, s) < 0) {\n"
    << indent << "  /* Found a more canonical representation. */\n"
    << indent << "  memcpy(s, &candidate, sizeof(*s));\n"
    << indent << "}\n\n"

    << indent << "{\n"
    << indent << "  size_t schedule_" << scalarset.name << "[" << bound << "] = { 0 };\n\n"

    << indent << "  for (size_t " << i << " = 0; " << i << " < " << bound << "; ) {\n"
    << indent << "    if (schedule_" << scalarset.name << "[" << i << "] < " << i << ") {\n"
    << indent << "      if (" << i << " % 2 == 0) {\n"
    << indent << "        swap_" << scalarset.name << "(&candidate, 0, " << i << ");\n"
    << indent << "      } else {\n"
    << indent << "        swap_" << scalarset.name << "(&candidate, schedule_"
      << scalarset.name << "[" << i << "], " << i << ");\n"
    << indent << "      }\n";
}

static void generate_loop_footer(const TypeDecl &scalarset, size_t index,
    size_t level, std::ostream &out) {

  const std::string indent(level * 2, ' ');

  auto s = dynamic_cast<const Scalarset*>(scalarset.value->resolve());
  assert(s != nullptr);

  const std::string i = "i" + std::to_string(index);

  out
    << indent << "      schedule_" << scalarset.name << "[" << i << "]++;\n"
    << indent << "      " << i << " = 0;\n"
    << indent << "    } else {\n"
    << indent << "      schedule_" << scalarset.name << "[" << i << "] = 0;\n"
    << indent << "      " << i << "++;\n"
    << indent << "    }\n"
    << indent << "  }\n"
    << indent << "}\n";
}

static void generate_loop(const std::vector<const TypeDecl*> &scalarsets,
    size_t index, size_t level, std::ostream &out) {

  if (index < scalarsets.size() - 1)
    generate_loop(scalarsets, index + 1, level, out);

  generate_loop_header(*scalarsets[index], index, level, out);

  if (index < scalarsets.size() - 1) {
    generate_loop(scalarsets, index + 1, level + 3, out);
  } else {
    const std::string indent((level + 3) * 2, ' ');

    out
      << indent << "if (state_cmp(&candidate, s) < 0) {\n"
      << indent << "  /* Found a more canonical representation. */\n"
      << indent << "  memcpy(s, &candidate, sizeof(*s));\n"
      << indent << "}\n\n";
  }

  generate_loop_footer(*scalarsets[index], index, level, out);
}

void generate_canonicalise(const Model &m, std::ostream &out) {

  // Find types eligible for use in canonicalisation
  const std::vector<const TypeDecl*> scalarsets = get_scalarsets(m);

  // Generate functions to swap state elements with respect to each scalarset
  for (const TypeDecl *t : scalarsets)
    generate_swap(m, out, *t);

  // Write the function prelude
  out
    << "static void state_canonicalise(struct state *s "
      << "__attribute__((unused))) {\n"
    << "\n"
    << "  assert(s != NULL && \"attempt to canonicalise NULL state\");\n"
    << "\n"
    << "  /* A state to store the current permutation we are considering. */\n"
    << "  static _Thread_local struct state candidate;\n"
    << "  memcpy(&candidate, s, sizeof(candidate));\n"
    << "\n";

  if (!scalarsets.empty())
    generate_loop(scalarsets, 0, 1, out);

  // Write the function coda
  out
    << "}\n\n";
}
