#include <cstddef>
#include "../../common/escape.h"
#include "generate.h"
#include <gmpxx.h>
#include <iostream>
#include <memory>
#include <rumur/rumur.h>
#include <string>
#include "symmetry-reduction.h"
#include "utils.h"
#include <vector>

using namespace rumur;

static std::string rule_name_string(const Rule &r, size_t index) {
  if (r.name == "")
    return std::to_string(index + 1);
  return "\\\"" + escape(r.name) + "\\\"";
}

void generate_model(std::ostream &out, const Model &m) {

  // Write out the symmetry reduction canonicalisation function
  generate_canonicalise(m, out);
  out << "\n\n";

  // Generate each defined constant.
  for (const Ptr<Decl> &d : m.decls) {
    if (isa<ConstDecl>(d)) {
      generate_decl(out, *d);
      out << ";\n";
    }
  }

  // Generate each defined function or procedure.
  for (const Ptr<Function> &f : m.functions) {
    generate_function(out, *f, m.decls);
    out << "\n\n";
  }

  /* Generate a set of flattened (non-hierarchical) rules. The purpose of this
   * is to essentially remove rulesets from the cases we need to deal with
   * below.
   */
  std::vector<Ptr<Rule>> flat_rules;
  for (const Ptr<Rule> &r : m.rules) {
    std::vector<Ptr<Rule>> rs = r->flatten();
    flat_rules.insert(flat_rules.end(), rs.begin(), rs.end());
  }

  // Write out the start state rules.
  {
    size_t index = 0;
    for (const Ptr<Rule> &r : flat_rules) {
      if (auto s = dynamic_cast<const StartState*>(r.get())) {
        out << "static bool startstate" << index << "(struct state *NONNULL s";
        for (const Quantifier &q : s->quantifiers)
          out << ", struct handle ru_" << q.name;
        out << ") {\n";

        out << "  static const char *rule_name __attribute__((unused)) = \"startstate "
          << rule_name_string(*s, index) << "\";\n";

        out
          << "  if (JMP_BUF_NEEDED) {\n"
          << "    if (sigsetjmp(checkpoint, 0)) {\n"
          << "      /* error triggered during this startstate */\n"
          << "      return false;\n"
          << "    }\n"
          << "  }\n";

        /* Output the state variable handles so we can reference them within
         * this start state.
         */
        for (const Ptr<Decl> &d : m.decls) {
          if (isa<VarDecl>(d)) {
            out << "  ";
            generate_decl(out, *d);
            out << ";\n";
          }
        }

        /* Output alias definitions, opening a scope in advance to support
         * aliases that shadow state variables, parameters, or other aliases.
         */
        for (const Ptr<AliasDecl> &a : s->aliases) {
          out << "   {\n  ";
          generate_decl(out, *a);
          out << ";\n";
        }

        /* Open a scope to support local declarations can shadow the state
         * variables.
         */
        out << "  {\n";

        for (const Ptr<Decl> &d : s->decls) {
          if (auto v = dynamic_cast<const VarDecl*>(d.get())) {
            out << "    ";
            generate_decl(out, *v);
            out << ";\n";
          }
        }

        // Allocate memory for any complex-returning functions we call
        generate_allocations(out, s->body);

        for (auto &st : s->body) {
          out << "    ";
          generate_stmt(out, *st);
          out << ";\n";
        }

        // Close the scopes we created.
        out
          << "  }\n"
          << std::string(s->aliases.size(), '}') << "\n"
          << "  return true;\n"
          << "}\n\n";
        index++;
      }
    }
  }

  // Write out the property rules.
  {
    size_t index = 0;
    for (const Ptr<Rule> &r : flat_rules) {
      if (auto i = dynamic_cast<const PropertyRule*>(r.get())) {
        out << "static __attribute__((unused)) bool property" << index
          << "(const struct state *NONNULL s";
        for (const Quantifier &q : i->quantifiers)
          out << ", struct handle ru_" << q.name;
        out << ") {\n";

        out << "  static const char *rule_name __attribute__((unused)) = \"property "
          << rule_name_string(*i, index) << "\";\n";

        /* Output the state variable handles so we can reference them within
         * this property.
         */
        for (const Ptr<Decl> &d : m.decls) {
          if (isa<VarDecl>(d)) {
            out << "  ";
            generate_decl(out, *d);
            out << ";\n";
          }
        }

        /* Output alias definitions, opening a scope in advance to support
         * aliases that shadow state variables, parameters, or other aliases.
         */
        for (const Ptr<AliasDecl> &a : i->aliases) {
          out << "   {\n  ";
          generate_decl(out, *a);
          out << ";\n";
        }

        out << "  return ";
        generate_property(out, i->property);
        out << ";\n"
          << std::string(i->aliases.size(), '}') << "\n"
          << "}\n\n";
        index++;
      }
    }
  }

  // Write out the regular rules.
  {
    size_t index = 0;
    for (const Ptr<Rule> &r : flat_rules) {
      if (auto s = dynamic_cast<const SimpleRule*>(r.get())) {

        // Write the guard
        out << "static int guard" << index << "(const struct state *NONNULL s "
          "__attribute__((unused))";
        for (const Quantifier &q : s->quantifiers)
          out << ", struct handle ru_" << q.name
            << " __attribute__((unused))";
        out << ") {\n";

        out << "  static const char *rule_name __attribute__((unused)) = \""
          << "guard of rule " << rule_name_string(*s, index) << "\";\n";

        out
          << "  if (JMP_BUF_NEEDED) {\n"
          << "    if (sigsetjmp(checkpoint, 0)) {\n"
          << "      /* this guard triggered an error */\n"
          << "      return -1;\n"
          << "    }\n"
          << "  }\n";

        /* Output the state variable handles so we can reference them within
         * this guard.
         */
        for (const Ptr<Decl> &d : m.decls) {
          if (isa<VarDecl>(d)) {
            out << "  ";
            generate_decl(out, *d);
            out << ";\n";
          }
        }

        /* Output alias definitions, opening a scope in advance to support
         * aliases that shadow state variables, parameters, or other aliases.
         */
        for (const Ptr<AliasDecl> &a : s->aliases) {
          out << "   {\n  ";
          generate_decl(out, *a);
          out << ";\n";
        }

        out << "  return ";
        if (s->guard == nullptr) {
          out << "true";
        } else {
          generate_rvalue(out, *s->guard);
        }
        out << " ? 1 : 0;\n"
          << std::string(s->aliases.size(), '}') << "\n"
          << "}\n\n";

        // Write the body
        out << "static bool rule" << index << "(struct state *NONNULL s";
        for (const Quantifier &q : s->quantifiers)
          out << ", struct handle ru_" << q.name;
        out << ") {\n";

        out << "  static const char *rule_name __attribute__((unused)) = \"rule "
          << rule_name_string(*s, index) << "\";\n";

        out
          << "  if (JMP_BUF_NEEDED) {\n"
          << "    if (sigsetjmp(checkpoint, 0)) {\n"
          << "      /* an error was triggered during this rule */\n"
          << "      return false;\n"
          << "    }\n"
          << "  }\n";

        /* Output the state variable handles so we can reference them within
         * this rule.
         */
        for (const Ptr<Decl> &d : m.decls) {
          if (isa<VarDecl>(d)) {
            out << "  ";
            generate_decl(out, *d);
            out << ";\n";
          }
        }

        /* Output alias definitions, opening a scope in advance to support
         * aliases that shadow state variables, parameters, or other aliases.
         */
        for (const Ptr<AliasDecl> &a : s->aliases) {
          out << "   {\n  ";
          generate_decl(out, *a);
          out << ";\n";
        }

        /* Open a scope to support local declarations can shadow the state
         * variables.
         */
        out << "  {\n";

        for (const Ptr<Decl> &d : s->decls) {
          if (isa<VarDecl>(d)) {
            out << "  ";
            generate_decl(out, *d);
            out << ";\n";
          }
        }

        // Allocate memory for any complex-returning functions we call
        generate_allocations(out, s->body);

        for (auto &st : s->body) {
          out << "  ";
          generate_stmt(out, *st);
          out << ";\n";
        }

        // Close the scopes we created.
        out
          << "  }\n"
          << std::string(s->aliases.size(), '}') << "\n"
          << "  return true;\n"
          << "}\n\n";

        index++;
      }
    }
  }

  // Write invariant checker
  {
    out
      << "static bool check_invariants(const struct state *NONNULL s "
        << "__attribute__((unused))) {\n"
      << "  static const char *rule_name __attribute__((unused)) = NULL;\n"
      << "  if (JMP_BUF_NEEDED) {\n"
      << "    if (sigsetjmp(checkpoint, 0)) {\n"
      << "      /* invariant violated */\n"
      << "      return false;\n"
      << "    }\n"
      << "  }\n";
    size_t index = 0;
    size_t invariant_index = 0;
    for (const Ptr<Rule> &r : flat_rules) {
      if (auto p = dynamic_cast<const PropertyRule*>(r.get())) {
        if (p->property.category == Property::ASSERTION) {

          // Open a scope so we don't have to think about name collisions.
          out << "  {\n";

          // Set up quantifiers.
          for (const Quantifier &q : r->quantifiers)
            generate_quantifier_header(out, q);

          out << "    if (!property" << index << "(s";
          for (const Quantifier &q : r->quantifiers)
            out << ", ru_" << q.name;
          out << ")) {\n"
            << "      error(s, \"invariant %s failed\", \""
              << rule_name_string(*p, invariant_index) << "\");\n"
            << "    }\n";

          // Close the quantifier loops.
          for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++)
            generate_quantifier_footer(out, *it);

          // Close this invariant's scope.
          out << "  }\n";

          invariant_index++;
        }
        index++;
      }
    }
    out
      << "  return true;\n"
      << "}\n\n";
  }

  // Write assumption checker
  {
    out
      << "static bool check_assumptions(const struct state *NONNULL s "
        << "__attribute__((unused))) {\n"
      << "  static const char *rule_name __attribute__((unused)) = NULL;\n"
      << "  if (JMP_BUF_NEEDED) {\n"
      << "    if (sigsetjmp(checkpoint, 0)) {\n"
      << "      /* one of the properties triggered an error */\n"
      << "      return false;\n"
      << "    }\n"
      << "  }\n";
    size_t index = 0;
    for (const Ptr<Rule> &r : flat_rules) {
      if (auto p = dynamic_cast<const PropertyRule*>(r.get())) {
        if (p->property.category == Property::ASSUMPTION) {

          // Open a scope so we don't have to think about name collisions.
          out << "  {\n";

          // Set up quantifiers.
          for (const Quantifier &q : r->quantifiers)
            generate_quantifier_header(out, q);

          out << "    if (!property" << index << "(s";
          for (const Quantifier &q : r->quantifiers)
            out << ", ru_" << q.name;
          out << ")) {\n"
            << "      /* Assumption violated. */\n"
            << "      return false;\n"
            << "    }\n";

          // Close the quantifier loops.
          for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++)
            generate_quantifier_footer(out, *it);

          // Close this invariant's scope.
          out << "  }\n";

        }
        index++;
      }
    }
    out
      << "  return true;\n"
      << "}\n\n";
  }

  // Write cover checker
  {
    out
      << "static bool check_covers(const struct state *NONNULL s "
        << "__attribute__((unused))) {\n"
      << "  static const char *rule_name __attribute__((unused)) = NULL;\n"
      << "  if (JMP_BUF_NEEDED) {\n"
      << "    if (sigsetjmp(checkpoint, 0)) {\n"
      << "      /* one of the properties triggered an error */\n"
      << "      return false;\n"
      << "    }\n"
      << "  }\n";
    size_t index = 0;
    for (const Ptr<Rule> &r : flat_rules) {
      if (auto p = dynamic_cast<const PropertyRule*>(r.get())) {
        if (p->property.category == Property::COVER) {

          // Open a scope so we don't have to think about name collisions.
          out << "  {\n";

          // Set up quantifiers.
          for (const Quantifier &q : r->quantifiers)
            generate_quantifier_header(out, q);

          out << "    if (property" << index << "(s";
          for (const Quantifier &q : r->quantifiers)
            out << ", ru_" << q.name;
          out << ")) {\n"
            << "      /* Covered. */\n"
            << "      (void)__atomic_fetch_add(&covers[COVER_"
              << p->property.unique_id << "], 1, __ATOMIC_SEQ_CST);\n"
            << "    }\n";

          // Close the quantifier loops.
          for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++)
            generate_quantifier_footer(out, *it);

          // Close this cover's scope.
          out << "  }\n";

        }
        index++;
      }
    }
    out
      << "  return true;\n"
      << "}\n\n";
  }

  // Write liveness checker
  {
    out
      << "#if LIVENESS_COUNT > 0\n"
      << "static bool check_liveness(struct state *NONNULL s "
        << "__attribute__((unused))) {\n"
      << "  static const char *rule_name __attribute__((unused)) = NULL;\n"
      << "  if (JMP_BUF_NEEDED) {\n"
      << "    if (sigsetjmp(checkpoint, 0)) {\n"
      << "      /* one of the liveness properties triggered an error */\n"
      << "      return false;\n"
      << "    }\n"
      << "  }\n"
      << "  size_t liveness_index __attribute__((unused)) = 0;\n";
    size_t index = 0;
    for (const Ptr<Rule> &r : flat_rules) {
      if (auto p = dynamic_cast<const PropertyRule*>(r.get())) {
        if (p->property.category == Property::LIVENESS) {

          // Open a scope so we don't have to think about name collisions.
          out << "  {\n";

          // Set up quantifiers.
          for (const Quantifier &q : r->quantifiers)
            generate_quantifier_header(out, q);

          out << "    if (property" << index << "(s";
          for (const Quantifier &q : r->quantifiers)
            out << ", ru_" << q.name;
          out << ")) {\n"
            << "      /* Hit. */\n"
            << "      mark_liveness(s, liveness_index, false);\n"
            << "    }\n"
            << "    liveness_index++;\n";

          // Close the quantifier loops.
          for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++)
            generate_quantifier_footer(out, *it);

          // Close this cover's scope.
          out << "  }\n";

        }
        index++;
      }
    }
    out
      << "  return true;\n"
      << "}\n\n";
  }

  // Write final liveness checker, the one that runs just prior to termination
  {
    out
      << "static void check_liveness_final(void) {\n"
      << "\n"
      << "  static const char *rule_name __attribute__((unused)) = NULL;\n"
      << "\n"
      << "  if (!MACHINE_READABLE_OUTPUT) {\n"
      << "    printf(\"trying to prove remaining liveness constraints...\\n\");\n"
      << "  }\n"
      << "\n"
      << "  /* find how many liveness bits are unknown */\n"
      << "  unsigned long remaining = 0;\n"
      << "  unsigned long long last_update = 0;\n"
      << "  unsigned long learned_since_last = 0;\n"
      << "  if (!MACHINE_READABLE_OUTPUT) {\n"
      << "    for (size_t i = 0; i < set_size(local_seen); i++) {\n"
      << "\n"
      << "      slot_t slot = __atomic_load_n(&local_seen->bucket[i], __ATOMIC_SEQ_CST);\n"
      << "\n"
      << "      ASSERT(!slot_is_tombstone(slot)\n"
      << "        && \"seen set being migrated during final liveness check\");\n"
      << "\n"
      << "      if (slot_is_empty(slot)) {\n"
      << "        /* skip empty entries in the hash table */\n"
      << "        continue;\n"
      << "      }\n"
      << "\n"
      << "      struct state *s = slot_to_state(slot);\n"
      << "      ASSERT(s != NULL && \"null pointer stored in state set\");\n"
      << "\n"
      << "      remaining += unknown_liveness(s);\n"
      << "    }\n"
      << "    printf(\"\\t %lu constraints remaining\\n\", remaining);\n"
      << "    last_update = gettime();\n"
      << "  }\n"
      << "\n"
      << "  bool progress = true;\n"
      << "  while (progress) {\n"
      << "    progress = false;\n"
      << "\n"
      << "    /* Run through all seen states trying to learn new liveness information. */\n"
      << "    for (size_t i = 0; i < set_size(local_seen); i++) {\n"
      << "\n"
      << "      slot_t slot = __atomic_load_n(&local_seen->bucket[i], __ATOMIC_SEQ_CST);\n"
      << "\n"
      << "      ASSERT(!slot_is_tombstone(slot)\n"
      << "        && \"seen set being migrated during final liveness check\");\n"
      << "\n"
      << "      if (slot_is_empty(slot)) {\n"
      << "        /* skip empty entries in the hash table */\n"
      << "        continue;\n"
      << "      }\n"
      << "\n"
      << "      struct state *s = slot_to_state(slot);\n"
      << "      ASSERT(s != NULL && \"null pointer stored in state set\");\n"
      << "\n"
      << "      if (unknown_liveness(s) == 0) {\n"
      << "        /* skip entries where liveness is fully satisfied already */\n"
      << "        continue;\n"
      << "      }\n"
      << "\n"
      << "#if BOUND > 0\n"
      << "      /* If we're doing bounded checking and this state is at the bound limit,\n"
      << "       * it's not valid to expand beyond this.\n"
      << "       */\n"
      << "      ASSERT(state_bound_get(s) <= BOUND && \"a state that exceeded the bound depth was explored\");\n"
      << "      if (state_bound_get(s) == BOUND) {\n"
      << "        continue;\n"
      << "      }\n"
      << "#endif\n"
      << "\n";
    size_t index = 0;
    for (const Ptr<Rule> &r : flat_rules) {
      if (isa<SimpleRule>(r)) {

        // Open a scope so we don't have to think about name collisions.
        out << "      {\n";

        for (const Quantifier &q : r->quantifiers)
          generate_quantifier_header(out, q);

        out
          // Use a dummy do-while to give us 'break' as a local goto.
          << "        do {\n"
          << "          struct state *n = state_dup(s);\n"
          << "\n"
          << "          int g = guard" << index << "(n";
        for (const Quantifier &q : r->quantifiers)
          out << ", ru_" << q.name;
        out << ");\n"
          << "          if (g == -1) {\n"
          << "            /* guard triggered an error */\n"
          << "            state_free(n);\n"
          << "            break;\n"
          << "          } else if (g == 1) {\n"
          << "            if (!rule" << index << "(n";
        for (const Quantifier &q : r->quantifiers)
          out << ", ru_" << q.name;
        out << ")) {\n"
          << "              /* this rule triggered an error */\n"
          << "              state_free(n);\n"
          << "              break;\n"
          << "            }\n"
          << "            state_canonicalise(n);\n"
          << "            if (!check_assumptions(n)) {\n"
          << "              /* assumption violated */\n"
          << "              state_free(n);\n"
          << "              break;\n"
          << "            }\n"
          << "\n"
          << "            /* note that we can skip an invariant check because we already know it\n"
          << "             * passed from prior expansion of this state.\n"
          << "             */\n"
          << "\n"
          << "            /* We should be able to find this state in the seen set. */\n"
          << "            const struct state *t = set_find(n);\n"
          << "            ASSERT(t != NULL && \"state encountered during final liveness wrap up \"\n"
          << "              \"that was not previously seen\");\n"
          << "\n"
          << "            /* See if this successor state learned a liveness property it never\n"
          << "             * passed back to us. This can occur if the state our exploration\n"
          << "             * encountered (`n`) was not the first of its kind seen and thus was\n"
          << "             * de-duped and never made it into the seen set with a back pointer\n"
          << "             * to `s`.\n"
          << "             */\n"
          << "            unsigned long learned = learn_liveness(s, t);\n"
          << "            if (learned > 0) {\n"
          << "              if (!MACHINE_READABLE_OUTPUT) {\n"
          << "                learned_since_last += learned;\n"
          << "                remaining -= learned;\n"
          << "                unsigned long long t = gettime();\n"
          << "                if (t > last_update) {\n"
          << "                  printf(\"\\t %lu further liveness constraints proved in %llus, with %s%lu%s remaining\\n\",\n"
          << "                    learned_since_last, t - last_update, green(), remaining, reset());\n"
          << "                  learned_since_last = 0;\n"
          << "                  last_update = t;\n"
          << "                }\n"
          << "              }\n"
          << "              progress = true;\n"
          << "            }\n"
          << "          }\n"
          << "          /* we don't need this state anymore. */\n"
          << "          state_free(n);\n"
          << "        } while (0);\n";

        // Close the quantifier loops.
        for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++)
          generate_quantifier_footer(out, *it);

        // Close this rule's scope.
        out << "}\n";

        index++;
      }
    }
    out
      << "    }\n"
      << "  }\n"
      << "}\n"
      << "\n"
      << "\n"
      << "static unsigned long check_liveness_summarise(void) {\n"
      << "\n"
      << "  /* We can now finally check whether all liveness properties were hit. */\n"
      << "  bool missed[LIVENESS_COUNT];\n"
      << "  memset(missed, 0, sizeof(missed));\n"
      << "  for (size_t i = 0; i < set_size(local_seen); i++) {\n"
      << "\n"
      << "    slot_t slot = __atomic_load_n(&local_seen->bucket[i], __ATOMIC_SEQ_CST);\n"
      << "\n"
      << "    ASSERT(!slot_is_tombstone(slot)\n"
      << "      && \"seen set being migrated during final liveness check\");\n"
      << "\n"
      << "    if (slot_is_empty(slot)) {\n"
      << "      /* skip empty entries in the hash table */\n"
      << "      continue;\n"
      << "    }\n"
      << "\n"
      << "    const struct state *s = slot_to_state(slot);\n"
      << "    ASSERT(s != NULL && \"null pointer stored in state set\");\n"
      << "\n"
      << "    size_t index __attribute__((unused)) = 0;\n";
    index = 0;
    for (const Ptr<Rule> &r : flat_rules) {
      if (auto p = dynamic_cast<const PropertyRule*>(r.get())) {
        if (p->property.category == Property::LIVENESS) {

          // Open a scope so we don't have to think about name collisions.
          out << "    {\n";

          // Set up quantifiers. Note, in this case they are not used.
          for (const Quantifier &q : r->quantifiers)
            generate_quantifier_header(out, q);

          out
            << "      size_t word_index = index / (sizeof(s->liveness[0]) * CHAR_BIT);\n"
            << "      size_t bit_index = index % (sizeof(s->liveness[0]) * CHAR_BIT);\n"
            << "      if (!missed[index] && !((s->liveness[word_index] >> bit_index) & 0x1)) {\n"
            << "        /* missed */\n"
            << "        missed[index] = true;\n"
            << "        if (MACHINE_READABLE_OUTPUT) {\n"
            << "          printf(\"<error includes_trace=\\\"%s\\\">\\n<message>liveness property \", "
              << "COUNTEREXAMPLE_TRACE == CEX_OFF ? \"false\" : \"true\");\n"
            << "          xml_printf(\"" << rule_name_string(*p, index) << "\");\n"
            << "          printf(\" violated</message>\\n\");\n"
            << "        } else {\n"
            << "          printf(\"\\t%s%sliveness property %s violated:%s\\n\", red(), bold(), \""
              << rule_name_string(*p, index) << "\", reset());\n"
            << "        }\n"
            << "        print_counterexample(s);\n"
            << "        if (MACHINE_READABLE_OUTPUT) {\n"
            << "          printf(\"</error>\\n\");\n"
            << "        }\n"
            << "      }\n"
            << "      index++;\n";

          // Close the quantifier loops.
          for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++)
            generate_quantifier_footer(out, *it);

          // Close this cover's scope.
          out << "    }\n";

          index++;
        }
      }
    }
    out
      << "  }\n"
      << "\n"
      << "  /* total up how many misses we saw */\n"
      << "  unsigned long total = 0;\n"
      << "  for (size_t i = 0; i < sizeof(missed) / sizeof(missed[0]); i++) {\n"
      << "    if (missed[i]) {\n"
      << "      total++;\n"
      << "    }\n"
      << "  }\n"
      << "\n"
      << "  return total;\n"
      << "}\n"
      << "#endif\n"
      << "\n";
  }

  // Write initialisation
  {
    out
      << "static void init(void) {\n"
      << "  static const char *rule_name __attribute__((unused)) = NULL;\n"
      << "  size_t queue_id = 0;\n"
      << "  uint64_t rule_taken = 1;\n";

    size_t index = 0;
    for (const Ptr<Rule> &r : flat_rules) {
      if (isa<StartState>(r)) {

        // Open a scope so we don't have to think about name collisions.
        out << "  {\n";

        /* Define the state variable because the code emitted for quantifiers
         * expects it. They do not need a non-NULL value.
         */
        out << "    struct state *s = NULL;\n";

        // Set up quantifiers.
        for (const Quantifier &q : r->quantifiers)
          generate_quantifier_header(out, q);

        out
          // Use a dummy do-while to give us 'break' as a local goto.
          << "    do {\n"

          << "      s = state_new();\n"
          << "      memset(s, 0, sizeof(*s));\n"
          << "#if COUNTEREXAMPLE_TRACE != CEX_OFF\n"
          << "      state_rule_taken_set(s, rule_taken);\n"
          << "#endif\n"
          << "      if (!startstate" << index << "(s";
        for (const Quantifier &q : r->quantifiers)
          out << ", ru_" << q.name;
        out << ")) {\n"
          << "        /* startstate triggered an error */\n"
          << "        state_free(s);\n"
          << "        break;\n"
          << "      }\n"
          << "      state_canonicalise(s);\n"
          << "      if (!check_assumptions(s)) {\n"
          << "        /* assumption violated */\n"
          << "        state_free(s);\n"
          << "        break;\n"
          << "      }\n"
          << "      if (!check_invariants(s)) {\n"
          << "        /* invariant violated */\n"
          << "        state_free(s);\n"
          << "        break;\n"
          << "      }\n"
          << "      size_t size;\n"
          << "      if (set_insert(s, &size)) {\n"
          << "        if (!check_covers(s)) {\n"
          << "          /* one of the cover properties triggered an error */\n"
          << "          break;\n"
          << "        }\n"
          << "#if LIVENESS_COUNT > 0\n"
          << "        if (!check_liveness(s)) {\n"
          << "          /* one of the liveness properties triggered an error */\n"
          << "          break;\n"
          << "        }\n"
          << "#endif\n"
          << "        (void)queue_enqueue(s, queue_id);\n"
          << "        queue_id = (queue_id + 1) % (sizeof(q) / sizeof(q[0]));\n"
          << "      } else {\n"
          << "        state_free(s);\n"
          << "      }\n"
          << "    } while (0);\n"
          << "    rule_taken++;\n";

        // Close the quantifier loops.
        for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++)
          generate_quantifier_footer(out, *it);

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
      << "static void explore(void) {\n"
      << "\n"
      << "  /* Used when writing to quantifier variables. */\n"
      << "  static const char *rule_name __attribute__((unused)) = NULL;\n"
      << "\n"
      << "  size_t last_queue_size = 0;\n"
      << "\n"
      << "  /* Identifier of the last queue we interacted with. */\n"
      << "  size_t queue_id = thread_id;\n"
      << "\n"
      << "  for (;;) {\n"
      << "\n"
      << "    if (THREADS > 1 && __atomic_load_n(&error_count,\n"
      << "        __ATOMIC_SEQ_CST) >= MAX_ERRORS) {\n"
      << "      /* Another thread found an error. */\n"
      << "      break;\n"
      << "    }\n"
      << "\n"
      << "    const struct state *s = queue_dequeue(&queue_id);\n"
      << "    if (s == NULL) {\n"
      << "      break;\n"
      << "    }\n"
      << "\n"
      << "    bool possible_deadlock = true;\n"
      << "    uint64_t rule_taken = 1;\n";
    size_t index = 0;
    for (const Ptr<Rule> &r : flat_rules) {
      if (isa<SimpleRule>(r)) {

        // Open a scope so we don't have to think about name collisions.
        out << "    {\n";

        for (const Quantifier &q : r->quantifiers)
          generate_quantifier_header(out, q);

        out
          // Use a dummy do-while to give us 'break' as a local goto.
          << "      do {\n"
          << "        struct state *n = state_dup(s);\n"
          << "#if COUNTEREXAMPLE_TRACE != CEX_OFF\n"
          << "        state_rule_taken_set(n, rule_taken);\n"
          << "#endif\n"
          << "        int g = guard" << index << "(n";
        for (const Quantifier &q : r->quantifiers)
          out << ", ru_" << q.name;
        out << ");\n"
          << "        if (g == -1) {\n"
          << "          /* error() was called */\n"
          << "          state_free(n);\n"
          << "          break;\n"
          << "        } else if (g == 1) {\n"
          << "          if (!rule" << index << "(n";
        for (const Quantifier &q : r->quantifiers)
          out << ", ru_" << q.name;
        out << ")) {\n"
          << "            /* this rule triggered an error */\n"
          << "            state_free(n);\n"
          << "            break;\n"
          << "          }\n"
          << "          rules_fired_local++;\n"
          << "          if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_STUTTERING || !state_eq(s, n)) {\n"
          << "            possible_deadlock = false;\n"
          << "          }\n"
          << "          state_canonicalise(n);\n"
          << "          if (!check_assumptions(n)) {\n"
          << "            /* assumption violated */\n"
          << "            state_free(n);\n"
          << "            break;\n"
          << "          }\n"
          << "          if (!check_invariants(n)) {\n"
          << "            /* invariant violated */\n"
          << "            state_free(n);\n"
          << "            break;\n"
          << "          }\n"
          << "          size_t size;\n"
          << "          if (set_insert(n, &size)) {\n"
          << "\n"
          << "            if (!check_covers(n)) {\n"
          << "              /* one of the cover properties triggered an error */\n"
          << "              break;\n"
          << "            }\n"
          << "#if LIVENESS_COUNT > 0\n"
          << "            if (!check_liveness(n)) {\n"
          << "              /* one of the liveness properties triggered an error */\n"
          << "              break;\n"
          << "            }\n"
          << "#endif\n"
          << "\n"
          << "#if BOUND > 0\n"
          << "            if (state_bound_get(n) < BOUND) {\n"
          << "#endif\n"
          << "            size_t queue_size = queue_enqueue(n, thread_id);\n"
          << "            queue_id = thread_id;\n"
          << "\n"
          << "            if (size % 10000 == 0 && print_trylock()) {\n"
          << "              if (MACHINE_READABLE_OUTPUT) {\n"
          << "                printf(\"<progress states=\\\"%zu\\\" "
            << "duration_seconds=\\\"%llu\\\" rules_fired=\\\"%\" PRIuMAX \"\\\" "
            << "queue_size=\\\"%zu\\\" thread_id=\\\"%zu\\\"/>\\n\", size, "
            << "gettime(), rules_fired_local, queue_size, thread_id);\n"
          << "              } else {\n"
          << "                printf(\"\\t \");\n"
          << "                if (THREADS > 1) {\n"
          << "                  printf(\"thread %zu: \", thread_id);\n"
          << "                }\n"
          << "                printf(\"%zu states explored in %llus, with %\" PRIuMAX \" rules \"\n"
          << "                  \"fired and %s%zu%s states in the queue.\\n\", size, gettime(),\n"
          << "                  rules_fired_local, queue_size > last_queue_size ? yellow() : green(),\n"
          << "                  queue_size, reset());\n"
          << "              }\n"
          << "              print_unlock();\n"
          << "              last_queue_size = queue_size;\n"
          << "            }\n"
          << "\n"
          << "            if (THREADS > 1 && thread_id == 0 && phase == WARMUP && queue_size > 20) {\n"
          << "              start_secondary_threads();\n"
          << "              phase = RUN;\n"
          << "            }\n"
          << "\n"
          << "#if BOUND > 0\n"
          << "            }\n"
          << "#endif\n"
          << "          } else {\n"
          << "            state_free(n);\n"
          << "          }\n"
          << "        } else {\n"
          << "          state_free(n);\n"
          << "        }\n"
          << "      } while (0);\n"
          << "      rule_taken++;\n";

        // Close the quantifier loops.
        for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++)
          generate_quantifier_footer(out, *it);

        // Close this rule's scope.
        out << "}\n";

        index++;
      }
    }
    out
      << "    /* If we did not toggle 'possible_deadlock' off by this point, we\n"
      << "     * have a deadlock.\n"
      << "     */\n"
      << "    if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_OFF && possible_deadlock) {\n"
      << "      deadlock(s);\n"
      << "    }\n"
      << "\n"
      << "  }\n"
      << "  exit_with(EXIT_SUCCESS);\n"
      << "}\n\n";
  }

  // Write a function to print the state.
  out << "static void state_print(const struct state *previous, const struct "
    << "state *NONNULL s) {\n";
  /* Output the state variable handles so we can reference them within this
   * function.
   */
  for (const Ptr<Decl> &d : m.decls) {
    if (isa<VarDecl>(d)) {
      out << "  ";
      generate_decl(out, *d);
      out << ";\n";
    }
  }
  for (const Ptr<Decl> &d : m.decls) {
    if (auto v = dynamic_cast<const VarDecl*>(d.get()))
      generate_print(out, *v->type, v->name, "ru_" + v->name, true, true);
  }
  out
    << "}\n\n";

  // Write a function to print state transitions.
  out
    << "static void print_transition(const struct state *NONNULL s "
      << "__attribute__((unused))) {\n"
    << "  ASSERT(s != NULL);\n"
    << "  static const char *rule_name __attribute__((unused)) = NULL;\n"
    << "#if COUNTEREXAMPLE_TRACE != CEX_OFF\n"
    << "\n"
    << "  ASSERT(state_rule_taken_get(s) != 0 && \"unknown state transition\");\n"
    << "\n";

  {
    out
      << "  if (state_previous_get(s) == NULL) {\n"
      << "    uint64_t rule_taken = 1;\n";

    mpz_class base = 1;

    size_t index = 0;
    for (const Ptr<Rule> &r : flat_rules) {
      if (isa<StartState>(r)) {

        // Set up quantifiers.
        out << "    {\n";
        for (const Quantifier &q : r->quantifiers)
          generate_quantifier_header(out, q);

        out
          << "  if (state_rule_taken_get(s) == rule_taken) {\n"
          << "    if (MACHINE_READABLE_OUTPUT) {\n"
          << "      printf(\"<transition>\");\n"
          << "      xml_printf(\"Startstate "
            << rule_name_string(*r, index) << "\");\n"
          << "    } else {\n"
          << "      printf(\"Startstate %s\", \"" << rule_name_string(*r, index)
            << "\");\n"
          << "    }\n";
        {
          size_t i = 0;
          for (const Quantifier &q : r->quantifiers) {
            out
              << "    {\n"
              << "      value_t v = (value_t)((rule_taken - " << base
                << ") / (1";
            size_t j = r->quantifiers.size() - 1;
            for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++) {
              if (i == j)
                break;
              out << " * " << it->count();
              j--;
            }
            out << ") % " << q.count() << ") + " << q.lower_bound() << ";\n"

              << "      if (MACHINE_READABLE_OUTPUT) {\n"
              << "        printf(\"<parameter name=\\\"\");\n"
              << "        xml_printf(\"" << q.name << "\");\n"
              << "        printf(\"\\\">\");\n"
              << "      } else {\n"
              << "        printf(\", %s: \", \"" << q.name << "\");\n"
              << "      }\n";

            const Ptr<TypeExpr> t = q.type->resolve();
            if (auto e = dynamic_cast<const Enum*>(t.get())) {
              size_t member_index = 0;
              for (const std::pair<std::string, location> &member : e->members) {
                out << "      ";
                if (member_index > 0)
                  out << "else ";
                out << "if (v == VALUE_C(" << member_index << ")) {\n"
                  << "        if (MACHINE_READABLE_OUTPUT) {\n"
                  << "          xml_printf(\"" << member.first << "\");\n"
                  << "        } else {\n"
                  << "          printf(\"%s\", \"" << member.first << "\");\n"
                  << "        }\n"
                  << "      }\n";
                member_index++;
              }
              out
                << "      else {\n"
                << "        ASSERT(!\"illegal value for " << q.name << "\");\n"
                << "      }\n";
            } else {
              out << "      printf(\"%\" PRIVAL, value_to_string(v));\n";
            }

            out
              << "      if (MACHINE_READABLE_OUTPUT) {\n"
              << "        printf(\"</parameter>\");\n"
              << "      }\n"
              << "    }\n";
            i++;
          }
        }
        out
          << "    if (MACHINE_READABLE_OUTPUT) {\n"
          << "      printf(\"</transition>\\n\");\n"
          << "    } else {\n"
          << "      printf(\" fired.\\n\");\n"
          << "    }\n"
          << "    return;\n"
          << "  }\n";

        out << "      rule_taken++;\n";

        // Close the quantifier loops.
        for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++)
          generate_quantifier_footer(out, *it);
        out << "    }\n";

        // update base for future comparison against rule_taken
        mpz_class inc = 1;
        for (const Quantifier &q : r->quantifiers) {
          inc *= q.count();
        }
        base += inc;

        index++;
      }
    }
  }

  {
    out
      << "  } else {\n"
      << "    uint64_t rule_taken = 1;\n";

    mpz_class base = 1;

    size_t index = 0;
    for (const Ptr<Rule> &r : flat_rules) {
      if (isa<SimpleRule>(r)) {

        // Set up quantifiers.
        out << "    {\n";
        for (const Quantifier &q : r->quantifiers)
          generate_quantifier_header(out, q);

        out
          << "  if (state_rule_taken_get(s) == rule_taken) {\n"
          << "    if (MACHINE_READABLE_OUTPUT) {\n"
          << "      printf(\"<transition>\");\n"
          << "      xml_printf(\"Rule " << rule_name_string(*r, index)
            << "\");\n"
          << "    } else {\n"
          << "      printf(\"Rule %s\", \"" << rule_name_string(*r, index)
            << "\");\n"
          << "    }\n";
        {
          size_t i = 0;
          for (const Quantifier &q : r->quantifiers) {
            out
              << "    {\n"
              << "      value_t v = (value_t)((rule_taken - " << base << ") / (1";
            size_t j = r->quantifiers.size() - 1;
            for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++) {
              if (i == j)
                break;
              out << " * " << it->count();
              j--;
            }
            out << ") % " << q.count() << ") + " << q.lower_bound() << ";\n"

              << "      if (MACHINE_READABLE_OUTPUT) {\n"
              << "        printf(\"<parameter name=\\\"\");\n"
              << "        xml_printf(\"" << q.name << "\");\n"
              << "        printf(\"\\\">\");\n"
              << "      } else {\n"
              << "        printf(\", %s: \", \"" << q.name << "\");\n"
              << "      }\n";

            const Ptr<TypeExpr> t = q.type->resolve();
            if (auto e = dynamic_cast<const Enum*>(t.get())) {
              size_t member_index = 0;
              for (const std::pair<std::string, location> &member : e->members) {
                out << "      ";
                if (member_index > 0)
                  out << "else ";
                out << "if (v == VALUE_C(" << member_index << ")) {\n"
                  << "        if (MACHINE_READABLE_OUTPUT) {\n"
                  << "          xml_printf(\"" << member.first << "\");\n"
                  << "        } else {\n"
                  << "          printf(\"%s\", \"" << member.first << "\");\n"
                  << "        }\n"
                  << "      }\n";
                member_index++;
              }
              out
                << "      else {\n"
                << "        ASSERT(!\"illegal value for " << q.name << "\");\n"
                << "      }\n";
            } else {
              out << "      printf(\"%\" PRIVAL, value_to_string(v));\n";
            }

            out
              << "      if (MACHINE_READABLE_OUTPUT) {\n"
              << "        printf(\"</parameter>\");\n"
              << "      }\n"
              << "    }\n";
            i++;
          }
        }
        out
          << "    if (MACHINE_READABLE_OUTPUT) {\n"
          << "      printf(\"</transition>\\n\");\n"
          << "    } else {\n"
          << "      printf(\" fired.\\n\");\n"
          << "    }\n"
          << "    return;\n"
          << "  }\n";

        out << "      rule_taken++;\n";

        // Close the quantifier loops.
        for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++)
          generate_quantifier_footer(out, *it);
        out << "    }\n";

        // update base for future comparison against rule_taken
        mpz_class inc = 1;
        for (const Quantifier &q : r->quantifiers) {
          inc *= q.count();
        }
        base += inc;

        index++;
      }
    }
  }

  out
    << "  }\n"
    << "\n"
    << "  /* give some helpful output for debugging problems with this function. */\n"
    << "  fprintf(stderr, \"no rule found to link to state at depth %zu\\n\", state_depth(s));\n"
    << "  ASSERT(!\"unreachable\");\n"
    << "#endif\n"
    << "}\n\n";

  // Generate a function used during debugging
  out
    << "static void state_print_field_offsets(void) {\n"
    << "  printf(\"\t* state struct is %zu-byte aligned\\n\", "
      "__alignof__(struct state));\n";
  for (const Ptr<Decl> &d : m.decls) {
    if (auto v = dynamic_cast<const VarDecl*>(d.get()))
      out << "  printf(\"\t* field %s is located at state offset " << v->offset
        << " bits\\n\", \"" << v->name << "\");\n";
  }
  out
    << "  printf(\"\\n\");\n"
    << "}\n\n";
}
