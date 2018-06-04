#include <algorithm>
#include <cstdint>
#include "location.hh"
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <rumur/Rule.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <unordered_set>
#include <vector>

namespace rumur {

Model::Model(std::vector<Decl*> &&decls_, std::vector<Rule*> &&rules_, const location &loc_):
  Node(loc_), decls(decls_), rules(rules_) {

  // Check all rule names are distinct.
  std::unordered_set<std::string> names;
  for (const Rule *r : rules) {
    if (r->name != "<unnamed>") {
      if (!names.insert(r->name).second)
        throw Error("duplicate rule name " + r->name, r->loc);
    }
  }
}

Model::Model(const Model &other):
  Node(other) {
  for (const Decl *d : other.decls)
    decls.push_back(d->clone());
  for (const Rule *r : other.rules)
    rules.push_back(r->clone());
}

Model &Model::operator=(Model other) {
  swap(*this, other);
  return *this;
}

void swap(Model &x, Model &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.decls, y.decls);
  swap(x.rules, y.rules);
}

Model *Model::clone() const {
  return new Model(*this);
}

uint64_t Model::size_bits() const {
  size_t s = 0;
  for (const Decl *d : decls) {
    if (auto v = dynamic_cast<const VarDecl*>(d))
      s += v->type->width();
  }
  return s;
}

Model::~Model() {
  for (Decl *d : decls)
    delete d;
  for (Rule *r : rules)
    delete r;
}

void Model::generate(std::ostream &out) const {

  // Write out constants
  for (const Decl *d : decls) {
    if (auto c = dynamic_cast<const ConstDecl*>(d)) {
      c->generate(out);
      out << ";\n";
    }
  }

  out << "\n";

  /* Generate a set of flattened (non-hierarchical) rules. The purpose of this
   * is to essentially remove rulesets from the cases we need to deal with
   * below.
   */
  std::vector<Rule*> flat_rules;
  for (const Rule *r : rules) {
    std::vector<Rule*> rs = r->flatten();
    flat_rules.insert(flat_rules.end(), rs.begin(), rs.end());
  }

  // Write out the start state rules.
  {
    size_t index = 0;
    for (const Rule *r : flat_rules) {
      if (auto s = dynamic_cast<const StartState*>(r)) {
        out << "static void startstate" << index << "(struct state *s";
        for (const Quantifier *q : s->quantifiers)
          out << ", struct handle ru_" << q->var->name;
        out << ") {\n";

        for (const Decl *d : s->decls) {
          if (auto v = dynamic_cast<const VarDecl*>(d))
            out << "  uint8_t _ru_" << v->name << "[BITS_TO_BYTES("
              << v->type->width() << ")] = { 0 };\n"
              << "  struct handle ru_" << v->name << " = { .base = _ru_"
              << v->name << ", .offset = 0, .width = SIZE_C("
              << v->type->width() << ") };\n";
        }

        for (const Stmt *st : s->body)
          out << "  " << *st << ";\n";
        out << "}\n\n";
        index++;
      }
    }
  }

  // Write out the invariant rules.
  {
    size_t index = 0;
    for (const Rule *r : flat_rules) {
      if (auto i = dynamic_cast<const Invariant*>(r)) {
        out << "static bool invariant" << index << "(const struct state *s) "
          "{\n  return ";
        i->guard->generate_rvalue(out);
        out << ";\n}\n\n";
        index++;
      }
    }
  }

  // Write out the regular rules.
  {
    size_t index = 0;
    for (const Rule *r : flat_rules) {
      if (auto s = dynamic_cast<const SimpleRule*>(r)) {

        // Write the guard
        out << "static bool guard" << index << "(const struct state *s";
        if (s->guard == nullptr)
          out << " __attribute__((unused))";
        for (const Quantifier *q : s->quantifiers)
          out << ", struct handle ru_" << q->var->name;
        out << ") {\n  return ";
        if (s->guard == nullptr) {
          out << "true";
        } else {
          s->guard->generate_rvalue(out);
        }
        out << ";\n}\n\n";

        // Write the body
        out << "static void rule" << index << "(struct state *s";
        for (const Quantifier *q : s->quantifiers)
          out << ", struct handle ru_" << q->var->name;
        out << ") {\n";

        for (const Decl *d : s->decls) {
          if (auto v = dynamic_cast<const VarDecl*>(d))
            out << "  uint8_t _ru_" << v->name << "[BITS_TO_BYTES("
              << v->type->width() << ")] = { 0 };\n"
              << "  struct handle ru_" << v->name << " = { .base = _ru_"
              << v->name << ", .offset = 0, .width = SIZE_C("
              << v->type->width() << ") };\n";
        }

        for (const Stmt *st : s->body)
          out << "  " << *st << ";\n";
        out << "}\n\n";

        index++;
      }
    }
  }

  // Write invariant checker
  {
    out << "static void check_invariants(const struct state *s __attribute__((unused))) {\n";
    size_t index = 0;
    for (const Rule *r : flat_rules) {
      if (dynamic_cast<const Invariant*>(r) != nullptr) {
        out
          << "  if (!invariant" << index << "(s)) {\n"
          << "    error(s, \"failed invariant\");\n"
          << "  }\n";
        index++;
      }
    }
    out << "}\n\n";
  }

  // Write initialisation
  {
    out << "static void init(void) {\n";
    size_t index = 0;
    for (const Rule *r : flat_rules) {
      if (dynamic_cast<const StartState*>(r) != nullptr) {

        // Open a scope so we don't have to think about name collisions.
        out << "  {\n";

        // Set up quantifiers.
        for (const Quantifier *q : r->quantifiers)
          out
            << "      for (value_t _ru1_" << q->var->name << " = "
              << q->var->type->lower_bound() << "; _ru1_" << q->var->name
              << " <= " << q->var->type->upper_bound() << "; _ru1_"
              << q->var->name << " += " << (q->step == nullptr ? "VALUE_C(1)" :
              "VALUE_C(" + std::to_string(q->step->constant_fold()) + ")")
              << ") {\n"
            << "        uint8_t _ru2_" << q->var->name << "[BITS_TO_BYTES("
              << q->var->type->width() << ")] = { 0 };\n"
            << "        struct handle ru_" << q->var->name
              << " = { .base = _ru2_" << q->var->name
              << ", .offset = 0, .width = SIZE_C(" << q->var->type->width()
              << ") };\n"
            << "        handle_write(ru_" << q->var->name << ", _ru1_"
              << q->var->name << ");\n";

        out
          << "    struct state *s = state_new();\n"
          << "    startstate" << index << "(s";
        for (const Quantifier *q : r->quantifiers)
          out << ", ru_" << q->var->name;
        out << ");\n"
          << "    check_invariants(s);\n"
          << "    size_t size;\n"
          << "    if (set_insert(s, &size)) {\n"
          << "      queue_enqueue(s);\n"
          << "    } else {\n"
          << "      free(s);\n"
          << "    }\n";

        // Close the quantifier loops.
        out << std::string(r->quantifiers.size(), '}') << "\n";

        // Close this startstate's scope.
        out << "  }\n";

        index++;
      }
    }
    out << "}\n\n";
  }

  // Write exploration logic
  {
    out
      << "static int explore(void) {\n"
      << "  for (;;) {\n"
      << "    struct state *s = queue_dequeue();\n"
      << "    if (s == NULL) {\n"
      << "      break;\n"
      << "    }\n";
    size_t index = 0;
    for (const Rule *r : flat_rules) {
      if (dynamic_cast<const SimpleRule*>(r) != nullptr) {

        // Open a scope so we don't have to think about name collisions.
        out << "    {\n";

        /* Set up quantifiers. It might be surprising to notice that there is an
         * extra level of indirection here. A variable 'x' results in loop
         * counter '_ru1_x', storage array '_ru2_x' and handle 'ru_x'. We use
         * three variables rather than two in order to avoid rules that modify
         * the ruleset parameters (uncommon) affecting the loop counter.
         */
        // TODO: some sanity checks on the emitted loop
        for (const Quantifier *q : r->quantifiers)
          out
            << "      for (value_t _ru1_" << q->var->name << " = "
              << q->var->type->lower_bound() << "; _ru1_" << q->var->name
              << " <= " << q->var->type->upper_bound() << "; _ru1_"
              << q->var->name << " += " << (q->step == nullptr ? "VALUE_C(1)" :
              "VALUE_C(" + std::to_string(q->step->constant_fold()) + ")")
              << ") {\n"
            << "        uint8_t _ru2_" << q->var->name << "[BITS_TO_BYTES("
              << q->var->type->width() << ")] = { 0 };\n"
            << "        struct handle ru_" << q->var->name
              << " = { .base = _ru2_" << q->var->name
              << ", .offset = 0, .width = SIZE_C(" << q->var->type->width()
              << ") };\n"
            << "        handle_write(ru_" << q->var->name << ", _ru1_"
              << q->var->name << ");\n";

        out << "      if (guard" << index << "(s";
        for (const Quantifier *q : r->quantifiers)
          out << ", ru_" << q->var->name;
        out << ")) {\n"
          << "        struct state *n = state_dup(s);\n"
          << "        rule" << index << "(n";
        for (const Quantifier *q : r->quantifiers)
          out << ", ru_" << q->var->name;
        out << ");\n"
          << "        check_invariants(n);\n"
          << "        size_t size;\n"
          << "        if (set_insert(n, &size)) {\n"
          << "          queue_enqueue(n);\n"
          << "          if (size % 10000 == 0) {\n"
          << "            print(\"%zu states seen in %llu seconds\\n\", size, gettime());\n"
          << "          }\n"
          << "        } else {\n"
          << "          free(n);\n"
          << "        }\n"
          << "      }\n";

        // Close the quantifier loops.
        out << std::string(r->quantifiers.size(), '}') << "\n";

        // Close this rule's scope.
        out << "}\n";

        index++;
      }
    }
    out
      << "  }\n"
      << "  return EXIT_SUCCESS;\n"
      << "}\n\n";
  }

  // Write a function to print the state.
  out
    << "static void state_print(const struct state *s) {\n"
    << "  // TODO\n"
    << "}\n\n";

  for (Rule *r : flat_rules)
    delete r;
}

bool Model::operator==(const Node &other) const {
  auto o = dynamic_cast<const Model*>(&other);
  if (o == nullptr)
    return false;
  for (auto it = decls.begin(), it2 = o->decls.begin(); ; it++, it2++) {
    if (it == decls.end()) {
      if (it2 != o->decls.end())
        return false;
      break;
    }
    if (it2 == o->decls.end())
      return false;
    if (**it != **it2)
      return false;
  }
  for (auto it = rules.begin(), it2 = o->rules.begin(); ; it++, it2++) {
    if (it == rules.end()) {
      if (it2 != o->rules.end())
        return false;
      break;
    }
    if (it2 == o->rules.end())
      return false;
    if (**it != **it2)
      return false;
  }
  return true;
}

}
