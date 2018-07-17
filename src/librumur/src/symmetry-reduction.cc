#include <cassert>
#include <iostream>
#include <rumur/Decl.h>
#include <rumur/log.h>
#include <rumur/Model.h>
#include <rumur/symmetry-reduction.h>
#include <rumur/traverse.h>
#include <rumur/TypeExpr.h>
#include <utility>
#include <vector>

namespace rumur {

// Find all the named scalarset declarations in a model.
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

/* Derive a metric for a given type based on its utility as a pivot component
 * (see below). In this scheme lower is better. The score is essentially "how
 * many times is a scalarset used in this type?"
 */
[[gnu::warn_unused_result]] static unsigned interdependence(const TypeExpr &t) {

  if (auto r = dynamic_cast<const Record*>(&t)) {
    unsigned s = 0;
    for (const VarDecl *f : r->fields)
      s += interdependence(*f->type);
    return s;
  }

  if (auto a = dynamic_cast<const Array*>(&t))
    return interdependence(*a->index_type) + interdependence(*a->element_type);

  if (auto s = dynamic_cast<const Scalarset*>(&t))
    return 1;

  if (auto i = dynamic_cast<const TypeExprID*>(&t))
    return interdependence(*i->referent);

  return 0;
}

// Generate part of a memcmp-style comparator
static void generate_compare(std::ostream &out, const std::string &offset_a,
  const std::string &offset_b, const TypeExpr &type, size_t depth = 0) {

  const TypeExpr *t = type.resolve();

  const std::string indent = std::string((depth + 1) * 2, ' ');

  if (t->is_simple()) {
    const std::string width = "SIZE_C(" + std::to_string(t->width()) + ")";
    out

      /* Open a scope so we don't need to think about redeclaring/shadowing 'x'
       * and 'y'.
       */
      << indent << "{\n"

      // Directly compare the two pieces of data
      << indent << "  value_t x = handle_read_raw((struct handle){ .base = "
        << "(uint8_t*)s->data, .offset = " << offset_a << ", .width = " << width
        << " });\n"
      << indent << "  value_t y = handle_read_raw((struct handle){ .base = "
        << "(uint8_t*)s->data, .offset = " << offset_b << ", .width = " << width
        << " });\n"
      << indent << "  if (x < y) {\n"
      << indent << "    return -1;\n"
      << indent << "  } else if (x > y) {\n"
      << indent << "    return 1;\n"
      << indent << "  }\n"
      << indent << "  /* Fall through to comparison of next item. */\n"

      // Close scope
      << indent << "}\n";

    return;
  }

  if (auto a = dynamic_cast<const Array*>(t)) {

    // The number of elements in this array as a C code string
    const std::string ub = "SIZE_C("
      + std::to_string(a->index_type->count() - 1) + ")";

    // The bit size of each array as a C code string
    const std::string width = "SIZE_C(" +
      std::to_string(a->element_type->width()) + ")";

    // Generate a loop to iterate over all the elements
    const std::string var = "i" + std::to_string(depth);
    out << indent << "for (size_t " << var << " = 0; " << var << " < " << ub
      << "; " << var << "++) {\n";

    // Generate code to compare each element
    const std::string off_a = offset_a + " + " + var + " * " + width;
    const std::string off_b = offset_b + " + " + var + " * " + width;
    generate_compare(out, off_a, off_b, *a->element_type, depth + 1);

    // Close the loop
    out << indent << "}\n";

    return;
  }

  if (auto r = dynamic_cast<const Record*>(t)) {

    std::string off_a = offset_a;
    std::string off_b = offset_b;

    for (const VarDecl *f : r->fields) {

      // Generate code to compare this field
      generate_compare(out, off_a, off_b, *f->type, depth);

      // Jump over this field to get the offset of the next field
      const std::string width = "SIZE_C(" + std::to_string(f->type->width()) +
        ")";
      off_a += " + " + width;
      off_b += " + " + width;
    }

    return;
  }

  assert(!"missed case in generate_compare");
}

// Generate application of a schedule
static void generate_apply_swap(std::ostream &out, const std::string &offset_a,
  const std::string &offset_b, const TypeExpr &type, size_t depth = 0) {

  const TypeExpr *t = type.resolve();

  const std::string indent = std::string((depth + 1) * 2, ' ');

  if (t->is_simple()) {

    out
      << indent << "if (" << offset_a << " != " << offset_b << ") {\n"
      << indent << "  value_t a = handle_read_raw((struct handle){ .base = "
        << "(uint8_t*)s->data, .offset = " << offset_a << ", .width = SIZE_C("
        << t->width() << ") });\n"
      << indent << "  value_t b = handle_read_raw((struct handle){ .base = "
        << "(uint8_t*)s->data, .offset = " << offset_b << ", .width = SIZE_C("
        << t->width() << ") });\n"
      << indent << "  handle_write_raw((struct handle){ .base = "
        << "(uint8_t*)s->data, .offset = " << offset_b << ", .width = SIZE_C("
        << t->width() << ") }, a);\n"
      << indent << "  handle_write_raw((struct handle){ .base = "
        << "(uint8_t*)s->data, .offset = " << offset_a << ", .width = SIZE_C("
        << t->width() << ") }, b);\n"
      << indent << "}\n";
    return;
  }

  if (auto a = dynamic_cast<const Array*>(t)) {
    const std::string var = "i" + std::to_string(depth);
    const std::string len = "SIZE_C("
      + std::to_string(a->index_type->count() - 1) + ")";
    const std::string width = "SIZE_C("
      + std::to_string(a->element_type->width()) + ")";

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

    for (const VarDecl *f : r->fields) {
      generate_apply_swap(out, off_a, off_b, *f->type, depth);

      off_a += " + SIZE_C(" + std::to_string(f->width()) + ")";
      off_b += " + SIZE_C(" + std::to_string(f->width()) + ")";
    }
    return;
  }

  assert(!"missed case in generate_apply_swap");
}

namespace {

  /* A "pivot," a collection of state components that we can sort at runtime to
   * provide a canonical ordering for a given scalarset type.
   */
  struct Pivot {

    struct Component {
      size_t offset;
      const TypeExpr *type;
      unsigned interdependence;
    };

    const Model *model;
    const TypeDecl *type;
    std::vector<Component> components;

    Pivot(const Model &m, const TypeDecl &type_): model(&m), type(&type_) { }

    Pivot(const Pivot&) = default;
    Pivot(Pivot&&) = default;
    Pivot &operator=(Pivot&&) = default;
    Pivot &operator=(const Pivot&) = default;

    void collect_components(void) {
      // Look through the model state to find suitable components.
      size_t offset = 0;
      for (const Decl *d : model->decls) {
        if (auto v = dynamic_cast<const VarDecl*>(d)) {
          consider_component(offset, *v->type, 0);
          offset += v->type->width();
        }
      }
    }

    virtual bool is_eligible_component(const TypeExpr &expr) const {
      // First, fully resolve the type in case it is a typedef.
      const TypeExpr *vtype = expr.resolve();

      // Check if it is an array...
      auto a = dynamic_cast<const Array*>(vtype);
      if (a == nullptr)
        return false;

      // ...and indexed on our type.
      auto t = dynamic_cast<const TypeExprID*>(a->index_type);
      if (t == nullptr)
        return false;
      if (t->name != type->name)
        return false;

      return true;
    }

    // Add a component to this pivot.
    void add_component(size_t offset, const TypeExpr &t, unsigned bias) {

      /* Find the representative type against which we'll score this TypeExpr on
       * interdependence. E.g. in the case of a scalarset-indexed array, we score
       * the TypeExpr based on the element type alone because we know the
       * (positive) effect of the index type.
       */
      const TypeExpr *vtype = t.resolve();
      if (auto a = dynamic_cast<const Array*>(vtype))
        vtype = a->element_type;

      /* Assess how much interaction this component has with other scalarsets. If
       * this itself is a scalarset, we know it is our own type and has no
       * interaction.
       */
      unsigned i;
      if (dynamic_cast<const Scalarset*>(vtype) != nullptr) {
        i = 0;
      } else {
        i = interdependence(*vtype);
      }
      i += bias;

      Component c = { offset, &t, i };

      /* Insert the component in order of increasing interdependence so that we
       * can output more optimal pivot code eventually.
       */
      for (auto it = components.begin(); it != components.end(); it++) {
        if (c.interdependence <= it->interdependence) {
          components.insert(it, c);
          return;
        }
      }

      /* The element to insert scored higher than every existing component. Insert
       * at the end.
       */
      components.emplace_back(c);
    }

    // Recursively find and add components
    virtual void consider_component(size_t offset, const TypeExpr &t, unsigned bias) {
      /* If this decl is an array that can participate in the pivot, add it and
       * we're done.
       */
      if (is_eligible_component(t)) {
        add_component(offset, t, bias);
        return;
      }

      // If this is an array, consider its element type.
      if (auto a = dynamic_cast<const Array*>(t.resolve())) {
        if (dynamic_cast<const Scalarset*>(a->index_type->resolve()) != nullptr)
          bias++;
        consider_component(offset, *a->element_type, bias);
      }

      // If this is a record, consider each of its fields.
      if (auto r = dynamic_cast<const Record*>(t.resolve())) {
        for (const VarDecl *f : r->fields) {
          consider_component(offset, *f->type, bias);
          offset += f->type->width();
        }
      }
    }

    bool useful(void) const {
      return !components.empty();
    }

    bool useless(void) const {
      return !useful();
    }

    /* Score this pivot as a whole based on its components. This is a measure of
     * how much reshuffling based on this pivot will degrade the ability of other
     * pivots. As with 'interdependence,' lower is better.
     */
    unsigned interference(void) const {
      unsigned s = 0;
      for (const Component &c : components)
        s += c.interdependence;
      return s;
    }

    void generate_schedule_define(std::ostream &out) const {
      auto s = dynamic_cast<const Scalarset&>(*type->value);
      int64_t b = s.bound->constant_fold();

        /* An array indicating how to sort the given scalarset. For example, if we
         * have a scalarset(4), this array may end up as { 3, 1, 0, 2 }. This
         * would indicate any other scalarset state data encountered should map 0
         * to 3, 1 to itself, 2 to 0, and 3 to 2.
         */
      out
        << "  size_t schedule_ru_" << type->name << "[" << b << "] "
          << "__attribute__((unused));\n";
    }

    virtual void generate_schedule_sort(std::ostream &out) const {
      const std::string schedule = "schedule_ru_" + type->name;

      out

        // Initialise the schedule array to describe the existing order
        << "  for (size_t i = 0; i < sizeof(" << schedule << ") / sizeof("
          << schedule << "[0]); i++) {\n"
        << "    " << schedule << "[i] = i;\n"
        << "  }\n"

        // Sort the schedule array
        << "  sort(compare_ru_" << type->name << ", " << schedule << ", s, 0, "
          << "sizeof(" << schedule << ") / sizeof(" << schedule
          << "[0]) - 1);\n";
    }

    void generate_schedule_apply(std::ostream &out) const {

      const std::string schedule = "schedule_ru_" + type->name;

      out
        << "  trace(TC_SYMMETRY_REDUCTION, \"symmetry reduction schedule for "
          << "%s:\", \"" << type->name << "\");\n"
        << "  for (size_t i = 0; i < sizeof(" << schedule << ") / sizeof("
          << schedule << "[0]); i++) {\n"
        << "    trace(TC_SYMMETRY_REDUCTION, \" %zu: %zu\", i, " << schedule
          << "[i]);\n"
        << "  }\n";

      std::string offset = "SIZE_C(0)";

      for (const Decl *d : model->decls) {
        if (auto v = dynamic_cast<const VarDecl*>(d)) {
          generate_apply(out, offset, *v->type, 0);

          offset += " + SIZE_C(" + std::to_string(v->width()) + ")";
        }
      }

    }

    void generate_apply(std::ostream &out, const std::string &offset,
      const TypeExpr &type_, size_t depth) const {

      const std::string schedule = "schedule_ru_" + type->name;
      const std::string indent = std::string((depth + 1) * 2, ' ');

      if (auto t = dynamic_cast<const TypeExprID*>(&type_)) {
        if (t->name == type->name) {
          out
            << indent << "{\n"
            << indent << "  value_t v = handle_read_raw((struct handle){ .base "
              << "= (uint8_t*)s->data, .offset = " << offset
              << ", .width = SIZE_C(" << t->width() << ") });\n"
            << indent << "  if (v != 0) {\n"
            << indent << "    handle_write_raw((struct handle){ .base = "
              << "(uint8_t*)s->data, .offset = " << offset
              << ", .width = SIZE_C(" << t->width() << ") }, " << schedule
              << "[v - 1] + 1);\n"
            << indent << "  }\n"
            << indent << "}\n";
          return;
        }
      }

      const TypeExpr *t = type_.resolve();

      if (auto a = dynamic_cast<const Array*>(t)) {

        const std::string width = "SIZE_C("
          + std::to_string(a->element_type->width()) + ")";

        const std::string var = "i" + std::to_string(depth);

        auto i = dynamic_cast<const TypeExprID*>(a->index_type);
        if (i != nullptr && i->name == type->name) {

          const std::string src = "source" + std::to_string(depth);
          const std::string dst = "destination" + std::to_string(depth);

          out
            << indent << "for (size_t " << var << " = 0; " << var
              << " < sizeof(" << schedule << ") / sizeof(" << schedule
              << "[0]); " << var << "++) {\n"
            << indent << "  size_t " << src << " = " << var << ";\n"
            << indent << "  size_t " << dst << " = " << schedule << "[" << src
              << "];\n"
            << indent << "  while (" << dst << " < " << src << ") {\n"
            << indent << "    " << dst << " = " << schedule << "[" << dst
              << "];\n"
            << indent << "  }\n"
            << indent << "  if (" << dst << " != " << src << ") {\n";

          const std::string off_a = offset + " + " + src + " * " + width;
          const std::string off_b = offset + " + " + dst + " * " + width;;

          generate_apply_swap(out, off_a, off_b, *a->element_type, depth + 2);

          out
            << indent << "  }\n"
            << indent << "}\n";

        }

        const std::string len = "SIZE_C("
          + std::to_string(a->index_type->count() - 1) + ")";

        out
          << indent << "for (size_t " << var << " = 0; " << var << " < "
            << len << "; " << var << "++) {\n";

        const std::string off = offset + " + " + var + " * " + width;

        generate_apply(out, off, *a->element_type, depth + 1);

        out << indent << "}\n";

        return;
      }

      if (auto r = dynamic_cast<const Record*>(t)) {
        std::string off = offset;

        for (const VarDecl *f : r->fields) {
          generate_apply(out, off, *f->type, depth);

          off += " + SIZE_C(" + std::to_string(f->width()) + ")";
        }

        return;
      }
    }

    virtual void generate_comparison(std::ostream &out) const {

      out
        << "static int compare_ru_" << type->name << "(const struct state *s, "
          << "size_t a, size_t b) {\n";

      for (const Component &c : components) {

        auto a = dynamic_cast<const Array*>(c.type);
        assert(a != nullptr && "non-array type in Pivot; maybe an inheritor of "
          "Pivot isn't overriding generate_comparison?");

        // Find the coordinates of this pivot component
        const std::string offset = "SIZE_C(" + std::to_string(c.offset) + ")";
        const std::string width = "SIZE_C(" +
          std::to_string(a->element_type->width()) + ")";
        const std::string offset_a = offset + " + a * " + width;
        const std::string offset_b = offset + " + b * " + width;

        // Generate comparison code for this component
        generate_compare(out, offset_a, offset_b, *a->element_type);
      }

      out
        /* Generated code will eventually fall through to this return statement
         * if all pivot components for the given scalarset values are equal.
         */
        << "  return 0;\n"
        << "}";
    }

    virtual void generate_checks(std::ostream &out) const {
      out
        << "  for (size_t i = 1; i < SIZE_C(" << (type->value->count() - 1)
          << "); i++) {\n"
        << "    assert(compare_ru_" << type->name << "(s, i - 1, i) <= 0 && "
          << "\"after applying " << type->name << " schedule, state is not "
          << "sorted\");\n"
        << "  }\n";
    }

    virtual ~Pivot(void) { }

    // Construct a pivot for the given scalarset declaration.
    static Pivot *derive(const Model &m, const TypeDecl &t);
  };

  struct TopLevelArrayPivot : public Pivot {
    using Pivot::Pivot;

    TopLevelArrayPivot &operator=(TopLevelArrayPivot&&) = default;
    TopLevelArrayPivot &operator=(const TopLevelArrayPivot&) = default;

    // Override consider_component to suppress recursion into arrays
    void consider_component(size_t offset, const TypeExpr &t, unsigned bias) final {
      /* If this decl is an array that can participate in the pivot, add it and
       * we're done.
       */
      if (is_eligible_component(t)) {
        add_component(offset, t, bias);
        return;
      }

      // If this is a record, consider each of its fields.
      if (auto r = dynamic_cast<const Record*>(t.resolve())) {
        for (const VarDecl *f : r->fields) {
          consider_component(offset, *f->type, bias);
          offset += f->type->width();
        }
      }
    }

    virtual ~TopLevelArrayPivot(void) { }

    static TopLevelArrayPivot *create(const Model &m, const TypeDecl &t) {
      auto p = new TopLevelArrayPivot(m, t);
      p->collect_components();
      return p;
    }
  };

  struct NestedArrayPivot : public Pivot {
    using Pivot::Pivot;

    NestedArrayPivot &operator=(NestedArrayPivot&&) = default;
    NestedArrayPivot &operator=(const NestedArrayPivot&) = default;

    virtual ~NestedArrayPivot(void) { }

    static NestedArrayPivot *create(const Model &m, const TypeDecl &t) {
      auto p = new NestedArrayPivot(m, t);
      p->collect_components();
      return p;
    }
  };

  struct FieldPivot : public Pivot {
    using Pivot::Pivot;

    FieldPivot &operator=(FieldPivot&&) = default;
    FieldPivot &operator=(const FieldPivot&) = default;

    bool is_eligible_component(const TypeExpr &expr) const final {

      // Check if this is a typedef...
      auto t = dynamic_cast<const TypeExprID*>(&expr);
      if (t == nullptr)
        return false;

      // ...and it's our type.
      if (t->name != type->name)
        return false;

      assert(dynamic_cast<const Scalarset*>(t->resolve()) != nullptr &&
        "a typedef of a non-scalarset type has the same name as a scalarset "
        "typedef");

      return true;
    }

    void generate_schedule_sort(std::ostream &out) const final {
      const std::string schedule = "schedule_ru_" + type->name;

      out
        // Initialise the schedule array full of "invalid" sentinels
        << "  for (size_t i = 0; i < sizeof(" << schedule << ") / sizeof("
          << schedule << "[0]); i++) {\n"
        << "    " << schedule << "[i] = SIZE_MAX;\n"
        << "  }\n"

        // Open a new scope so we don't need to care about aliasing/shadowing
        << "  {\n"

        // Counter of how many unique values of our type we've seen
        << "    size_t i = 0;\n";

      for (const Component &c : components)
        out

          // Another scope so we can re-use 'v' for each component
          << "    {\n"

          // Read the (raw) value of this field
          << "      size_t v = (size_t)handle_read_raw((struct handle){ "
            << ".base = (uint8_t*)s->data, .offset = SIZE_C(" << c.offset
            << "), .width = SIZE_C(" << c.type->width() << ") });\n"

          << "      assert((v == 0 || v - 1 < sizeof(" << schedule
            << ") / sizeof(" << schedule << "[0])) && \"out of bounds access "
            << "in state_canonicalise()\");\n"

          /* If it's not undefined and its corresponding entry in the schedule
           * array is invalid, claim it.
           */
          << "      if (v != 0 && " << schedule << "[v - 1] != SIZE_MAX) {\n"
          << "        " << schedule << "[v - 1] = i;\n"
          << "        i++;\n"
          << "      }\n"

          // Close 'v' scope
          << "    }\n";

      out
        /* Use the remaining values of this type linearly to set the remaining
         * invalid slots.
         */
        << "    for (size_t j = 0; j < sizeof(" << schedule << ") / sizeof(" << schedule << "[0]); j++) {\n"
        << "      if (" << schedule << "[j] == SIZE_MAX) {\n"
        << "        " << schedule << "[j] = i;\n"
        << "        i++;\n"
        << "      }\n"
        << "    }\n"

        // Close our scope
        << "  }\n";
    }

    /* A field pivot needs no comparator, as it relies on ordering based on
     * offsets into the state data.
     */
    void generate_comparison(std::ostream&) const final { }

    void generate_checks(std::ostream&) const final { }

    virtual ~FieldPivot(void) { }

    static FieldPivot *create(const Model &m, const TypeDecl &t) {
      auto p = new FieldPivot(m, t);
      p->collect_components();
      return p;
    }
  };

  Pivot *Pivot::derive(const Model &m, const TypeDecl &t) {
    assert(dynamic_cast<const Scalarset*>(t.value) != nullptr &&
      "non-scalarset typedecl passed to Pivot::derive");

    Pivot *p = nullptr;

    // Preference 1: a top-level array pivot
    if (p == nullptr) {
      p = TopLevelArrayPivot::create(m, t);
      if (p->useless()) {
        delete p;
        p = nullptr;
      } else {
        *log.debug << __func__ << "():" << __LINE__ << ": symmetry reduction: "
          << "scalarset type " << t.name << " assigned a top-level array "
          << "pivot\n";
      }
    }

    // Preference 2: a nested array pivot
    if (p == nullptr) {
      p = NestedArrayPivot::create(m, t);
      if (p->useless()) {
        delete p;
        p = nullptr;
      } else {
        *log.debug << __func__ << "():" << __LINE__ << ": symmetry reduction: "
          << "scalarset type " << t.name << " assigned a nested array pivot\n";
      }
    }

    // Preference 3: an individual field pivot
    if (p == nullptr) {
      p = FieldPivot::create(m, t);
      if (p->useless()) {
        delete p;
        p = nullptr;
      } else {
        *log.debug << __func__ << "():" << __LINE__ << ": symmetry reduction: "
          << "scalarset type " << t.name << " assigned an individual field "
          << "pivot\n";
      }
    }

    // It's possible all the above failed and we end up returning NULL.
    if (p == nullptr)
      *log.debug << __func__ << "():" << __LINE__ << ": symmetry reduction: "
        << "scalarset type " << t.name << " could not be assigned a pivot\n";

    return p;
  }
}

void generate_canonicalise(const Model &m, std::ostream &out) {

  // Find the named scalarset types.
  std::vector<const TypeDecl*> ss = find_scalarsets(m);
  *log.info << "symmetry reduction: " << ss.size() << " eligible scalarset "
    "types\n";

  /* Derive a pivot for each one, keeping the list sorted by ascending
   * interference.
   */
  std::vector<Pivot*> pivots;
  for (const TypeDecl *t : ss) {
    Pivot *p = Pivot::derive(m, *t);
    if (p == nullptr) {
      *log.warn << "scalarset type " << t->name << " does not seem to be used "
        << "in the state and will be ignored during symmetry reduction\n";
      continue;
    }
    *log.debug << __func__ << "():" << __LINE__ << ": symmetry reduction: "
      << "pivot for scalarset type " << t->name << " has an interference score "
      << "of " << p->interference()
      << (p->interference() == 0 ? " (perfect)" : "") << "\n";
    bool found = false;
    for (auto it = pivots.begin(); it != pivots.end(); it++) {
      if (p->interference() <= (*it)->interference()) {
        pivots.insert(it, p);
        found = true;
        break;
      }
    }
    if (!found)
      pivots.push_back(p);
  }

  // Output a comparison function to be used in each pivot
  for (const Pivot *p : pivots) {
    p->generate_comparison(out);
    out << "\n\n";
  }

  // Write the function header
  out
    << "static void state_canonicalise(struct state *s "
      << "__attribute__((unused))) {\n";

  // Output code to perform the actual canonicalisation
  for (const Pivot *p : pivots) {
    out << "  /* Symmetry reduction for type " << p->type->name << " */\n";
    p->generate_schedule_define(out);
    p->generate_schedule_sort(out);
    p->generate_schedule_apply(out);
    p->generate_checks(out);
  }

  out << "}";

  for (Pivot *p : pivots)
    delete p;
}

}
