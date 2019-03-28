#include <cstddef>
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

void generate_model(std::ostream &out, const Model &m) {

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
        out << "static void startstate" << index << "(struct state *s";
        for (const Quantifier &q : s->quantifiers)
          out << ", struct handle ru_" << q.name;
        out << ") {\n";

        out << "  static const char *rule_name __attribute__((unused)) = \"startstate "
          << (s->name == "" ? std::to_string(index) : escape(s->name))
          << "\";\n";

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
        out << "static __attribute__((unused)) bool property" << index << "(const struct state *s";
        for (const Quantifier &q : i->quantifiers)
          out << ", struct handle ru_" << q.name;
        out << ") {\n";

        out << "  static const char *rule_name __attribute__((unused)) = \"property "
          << (i->name == "" ? std::to_string(index) : escape(i->name))
          << "\";\n";

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
        out << "static bool guard" << index << "(const struct state *s "
          "__attribute__((unused))";
        for (const Quantifier &q : s->quantifiers)
          out << ", struct handle ru_" << q.name
            << " __attribute__((unused))";
        out << ") {\n";

        out << "  static const char *rule_name __attribute__((unused)) = \"guard of "
          << (s->name == "" ? std::to_string(index) : escape(s->name))
          << "\";\n";

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
        out << ";\n"
          << std::string(s->aliases.size(), '}') << "\n"
          << "}\n\n";

        // Write the body
        out << "static void rule" << index << "(struct state *s";
        for (const Quantifier &q : s->quantifiers)
          out << ", struct handle ru_" << q.name;
        out << ") {\n";

        out << "  static const char *rule_name __attribute__((unused)) = \"rule "
          << (s->name == "" ? std::to_string(index) : escape(s->name))
          << "\";\n";

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
          << "}\n\n";

        index++;
      }
    }
  }

  // Write invariant checker
  {
    out
      << "static void check_invariants(const struct state *s __attribute__((unused))) {\n"
      << "  static const char *rule_name __attribute__((unused)) = NULL;\n";
    size_t index = 0;
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
            << "      error(s, false, \"invariant "
              << (p->name == "" ? std::to_string(index + 1) : "\\\"" + p->name + "\\\"") << " failed\");\n"
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
    out << "}\n\n";
  }

  // Write assumption checker
  {
    out
      << "static void check_assumptions(const struct state *s __attribute__((unused))) {\n"
      << "  static const char *rule_name __attribute__((unused)) = NULL;\n";
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
            << "      assert(JMP_BUF_NEEDED && \"longjmping without a setup jmp_buf\");\n"
            << "      siglongjmp(checkpoint, 1);\n"
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
    out << "}\n\n";
  }

  // Write cover checker
  {
    out
      << "static void check_covers(const struct state *s __attribute__((unused))) {\n"
      << "  static const char *rule_name __attribute__((unused)) = NULL;\n";
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
    out << "}\n\n";
  }

  // Write liveness checker
  {
    out
      << "static void check_liveness(struct state *s __attribute__((unused))) {\n"
      << "  static const char *rule_name __attribute__((unused)) = NULL;\n"
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
    out << "}\n\n";
  }

  // Write final liveness checker, the one that runs just prior to termination
  {
    out
      << "static unsigned long check_liveness_final(size_t *chunk_ptr) {\n"
      << "\n"
      << "  /* make it very obvious to compilers that this function is no-op\n"
      << "   * when we have no liveness checks.\n"
      << "   */\n"
      << "  if (LIVENESS_COUNT == 0) {\n"
      << "    return 0;\n"
      << "  }\n"
      << "\n"
      << "  static const char *rule_name __attribute__((unused)) = NULL;\n"
      << "\n"
      << "  enum { CHUNK_SIZE = 4096 / sizeof(local_seen->bucket[0]) /* slots */ };\n"
      << "\n"
      << "  unsigned long progress = 0;\n"
      << "\n"
      << "  for (;;) {\n"
      << "\n"
      << "    size_t chunk = __atomic_fetch_add(chunk_ptr, 1, __ATOMIC_SEQ_CST);\n"
      << "    size_t start = chunk * CHUNK_SIZE;\n"
      << "    size_t end = start + CHUNK_SIZE;\n"
      << "\n"
      << "    /* bail out if we're at the end of the set */\n"
      << "    if (start >= set_size(local_seen)) {\n"
      << "      break;\n"
      << "    }\n"
      << "\n"
      << "    /* Run through all seen states trying to learn new liveness information. */\n"
      << "    for (size_t i = start; i < end; i++) {\n"
      << "\n"
      << "      slot_t slot = __atomic_load_n(&local_seen->bucket[i], __ATOMIC_SEQ_CST);\n"
      << "\n"
      << "      if (slot_is_empty(slot)) {\n"
      << "        /* skip empty entries in the hash table */\n"
      << "        continue;\n"
      << "      }\n"
      << "\n"
      << "      struct state *s = slot_to_state(slot);\n"
      << "      ASSERT(s != NULL && \"null pointer stored in state set\");\n"
      << "\n"
      << "      if (known_liveness(s)) {\n"
      << "        /* skip entries where liveness is fully satisfied already */\n"
      << "        continue;\n"
      << "      }\n"
      << "\n"
      << "#if BOUND > 0\n"
      << "      /* If we're doing bounded checking and this state is at the bound limit,\n"
      << "       * it's not valid to expand beyond this.\n"
      << "       */\n"
      << "      ASSERT(s->bound <= BOUND && \"a state that exceeded the bound depth was explored\");\n"
      << "      if (s->bound == BOUND) {\n"
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
          << "          if (JMP_BUF_NEEDED) {\n"  // FIXME: This will use a jmp_buf even if we have no assumptions, and hence a jmp_buf would not be needed here
          << "            if (sigsetjmp(checkpoint, 0)) {\n"
          << "              /* assumption hit violated. */\n"
          << "              state_free(n);\n"
          << "              break;\n"
          << "            }\n"
          << "          }\n"
          << "          if (guard" << index << "(n";
        for (const Quantifier &q : r->quantifiers)
          out << ", ru_" << q.name;
        out << ")) {\n"
          << "            rule" << index << "(n";
        for (const Quantifier &q : r->quantifiers)
          out << ", ru_" << q.name;
        out << ");\n"
          << "            state_canonicalise(n);\n"
          << "            check_assumptions(n);\n"
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
          << "            progress += learn_liveness(s, t);\n"
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
      << "\n"
      << "  return progress;\n"
      << "}\n"
      << "\n"
      << "\n"
      << "static unsigned long check_liveness_summarize(void) {\n"
      << "\n"
      << "  /* We can now finally check whether all liveness properties were hit. */\n"
      << "  bool missed[LIVENESS_COUNT];\n"
      << "  memset(missed, 0, sizeof(missed));\n"
      << "  for (size_t i = 0; i < set_size(local_seen); i++) {\n"
      << "\n"
      << "    if (slot_is_empty(local_seen->bucket[i])) {\n"
      << "      /* skip empty entries in the hash table */\n"
      << "      continue;\n"
      << "    }\n"
      << "\n"
      << "    const struct state *s = slot_to_state(local_seen->bucket[i]);\n"
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
            << "          char *msg = xml_escape(\""
              << (p->name == "" ? std::to_string(index + 1) : "\\\"" + escape(p->name) + "\\\"") << "\");\n"
            << "          printf(\"<error><message>liveness property %s violated</message>\", msg);\n"
            << "          free(msg);\n"
            << "        } else {\n"
            << "          printf(\"\\t%s%sliveness property %s violated:%s\\n\", red(), bold(), \""
              << (p->name == "" ? std::to_string(index + 1) : "\\\"" + escape(p->name) + "\\\"") << "\", reset());\n"
            << "        }\n"
            << "        state_print(NULL, s);\n"
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
      << "}\n\n";
  }

  // Write out the symmetry reduction canonicalisation function
  generate_canonicalise(m, out);
  out << "\n\n";

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
          << "      s->rule_taken = rule_taken;\n"
          << "#endif\n"
          << "      if (JMP_BUF_NEEDED) {\n"
          << "        if (sigsetjmp(checkpoint, 0)) {\n"
          << "          /* error() was called. */\n"
          << "          break;\n"
          << "        }\n"
          << "      }\n"
          << "      startstate" << index << "(s";
        for (const Quantifier &q : r->quantifiers)
          out << ", ru_" << q.name;
        out << ");\n"
          << "      state_canonicalise(s);\n"
          << "      check_assumptions(s);\n"
          << "      check_invariants(s);\n"
          << "      size_t size;\n"
          << "      if (set_insert(s, &size)) {\n"
          << "        check_covers(s);\n"
          << "        check_liveness(s);\n"
          << "        (void)queue_enqueue(s, queue_id);\n"
          << "        queue_id = (queue_id + 1) % (sizeof(q) / sizeof(q[0]));\n"
          << "        s = NULL;\n"
          << "      }\n"
          << "      state_free(s);\n"
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
          << "        n->rule_taken = rule_taken;\n"
          << "#endif\n"
          << "        if (JMP_BUF_NEEDED) {\n"
          << "          if (sigsetjmp(checkpoint, 0)) {\n"
          << "            /* error() was called. */\n"
          << "            break;\n"
          << "          }\n"
          << "        }\n"
          << "        if (guard" << index << "(n";
        for (const Quantifier &q : r->quantifiers)
          out << ", ru_" << q.name;
        out << ")) {\n"
          << "          rule" << index << "(n";
        for (const Quantifier &q : r->quantifiers)
          out << ", ru_" << q.name;
        out << ");\n"
          << "          rules_fired_local++;\n"
          << "          if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_STUTTERING || !state_eq(s, n)) {\n"
          << "            possible_deadlock = false;\n"
          << "          }\n"
          << "          state_canonicalise(n);\n"
          << "          check_assumptions(n);\n"
          << "          check_invariants(n);\n"
          << "          size_t size;\n"
          << "          if (set_insert(n, &size)) {\n"
          << "\n"
          << "            check_covers(n);\n"
          << "            check_liveness(n);\n"
          << "\n"
          << "#if BOUND > 0\n"
          << "            if (n->bound < BOUND) {\n"
          << "#endif\n"
          << "            size_t queue_size = queue_enqueue(n, thread_id);\n"
          << "            queue_id = thread_id;\n"
          << "\n"
          << "            if (size % 10000 == 0) {\n"
          << "              print_lock();\n"
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
          << "\n"
          << "            /* Avoid freeing the state we just added to the set. */\n"
          << "            n = NULL;\n"
          << "          }\n"
          << "        }\n"
          << "        state_free(n);\n"
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
      << "      error(s, true, \"deadlock\");\n"
      << "    }\n"
      << "\n"
      << "  }\n"
      << "  exit_with(EXIT_SUCCESS);\n"
      << "}\n\n";
  }

  // Write a function to print the state.
  out
    << "static void state_print(const struct state *previous, const struct state *s) {\n";
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
    << "static void print_transition(const struct state *s __attribute__((unused))) {\n"
    << "  ASSERT(s != NULL);\n"
    << "  static const char *rule_name __attribute__((unused)) = NULL;\n"
    << "#if COUNTEREXAMPLE_TRACE != CEX_OFF\n"
    << "\n"
    << "  if (s->rule_taken == 0) {\n"
    << "    fprintf(stderr, \"unknown state transition\\n\");\n"
    << "    ASSERT(s->rule_taken != 0 && \"unknown state transition\");\n"
    << "    return;\n"
    << "  }\n"
    << "\n";

  {
    out
      << "  if (s->previous == NULL) {\n"
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
          << "  if (s->rule_taken == rule_taken) {\n"
          << "    if (MACHINE_READABLE_OUTPUT) {\n"
          << "      printf(\"<transition>\");\n"
          << "      char *escaped_name = xml_escape(\""
            << (r->name == "" ? "Startstate " + std::to_string(index) : r->name)
            << "\");\n"
          << "      printf(\"%s\", escaped_name);\n"
          << "      free(escaped_name);\n"
          << "    } else {\n"
          << "      printf(\"Startstate %s\", \""
            << (r->name == "" ? "Startstate " + std::to_string(index) : r->name)
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
              << "        char *escaped_name = xml_escape(\"" << q.name
                << "\");\n"
              << "        printf(\"<parameter name=\\\"%s\\\">\", "
                << "escaped_name);\n"
              << "        free(escaped_name);\n"
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
                  << "          char *escaped_name = xml_escape(\""
                    << member.first << "\");\n"
                  << "          printf(\"%s\", escaped_name);\n"
                  << "          free(escaped_name);\n"
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
              out << "      printf(\"%s\", value_to_string(v).data);\n";
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
          << "  if (s->rule_taken == rule_taken) {\n"
          << "    if (MACHINE_READABLE_OUTPUT) {\n"
          << "      printf(\"<transition>\");\n"
          << "      char *escaped_name = xml_escape(\""
            << (r->name == "" ? "Rule " + std::to_string(index) : r->name)
            << "\");\n"
          << "      printf(\"%s\", escaped_name);\n"
          << "      free(escaped_name);\n"
          << "    } else {\n"
          << "      printf(\"Rule %s\", \""
            << (r->name == "" ? "Rule " + std::to_string(index) : r->name)
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
              << "        char *escaped_name = xml_escape(\"" << q.name
                << "\");\n"
              << "        printf(\"<parameter name=\\\"%s\\\">\", "
                << "escaped_name);\n"
              << "        free(escaped_name);\n"
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
                  << "          char *escaped_name = xml_escape(\""
                    << member.first << "\");\n"
                  << "          printf(\"%s\", escaped_name);\n"
                  << "          free(escaped_name);\n"
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
              out << "      printf(\"%s\", value_to_string(v).data);\n";
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
  out << "static void state_print_field_offsets(void) {\n";
  for (const Ptr<Decl> &d : m.decls) {
    if (auto v = dynamic_cast<const VarDecl*>(d.get()))
      out << "  printf(\"\t* field %s is located at state offset " << v->offset
        << " bits\\n\", \"" << v->name << "\");\n";
  }
  out
    << "  printf(\"\\n\");\n"
    << "}\n\n";
}
