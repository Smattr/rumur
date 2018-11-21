#include <cstddef>
#include "generate.h"
#include <gmpxx.h>
#include <iostream>
#include <memory>
#include <rumur/rumur.h>
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
    out << "static void check_invariants(const struct state *s __attribute__((unused))) {\n";
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
            << "      error(s, false, \"failed invariant\");\n"
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
    out << "static void check_assumptions(const struct state *s __attribute__((unused))) {\n";
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
            << "      longjmp(checkpoint, 1);\n"
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

  // Write out the symmetry reduction canonicalisation function
  generate_canonicalise(m, out);
  out << "\n\n";

  // Write initialisation
  {
    out
      << "static void init(void) {\n"
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
          << "#if COUNTEREXAMPLE_TRACE != CEX_OFF\n"
          << "      s->rule_taken = rule_taken;\n"
          << "#endif\n"
          << "      if (JMP_BUF_NEEDED) {\n"
          << "        if (setjmp(checkpoint)) {\n"
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
          << "        (void)queue_enqueue(s, queue_id);\n"
          << "        queue_id = (queue_id + 1) % (sizeof(q) / sizeof(q[0]));\n"
          << "        s = NULL;\n"
          << "      }\n"
          << "      free(s);\n"
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
      << "  size_t last_queue_size = 0;\n"
      << "\n"
      << "  /* Identifier of the last queue we interacted with. */\n"
      << "  size_t queue_id = thread_id;\n"
      << "\n"
      << "  for (;;) {\n"
      << "\n"
      << "    if (THREADS > 1 && error_count >= MAX_ERRORS) {\n"
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
          << "          if (setjmp(checkpoint)) {\n"
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
          << "            size_t queue_size = queue_enqueue(n, thread_id);\n"
          << "            queue_id = thread_id;\n"
          << "\n"
          << "            if (size % 10000 == 0) {\n"
          << "              print_lock();\n"
          << "              if (MACHINE_READABLE_OUTPUT) {\n"
          << "                printf(\"<progress states=\\\"%zu\\\" "
            << "duration_seconds=\\\"%llu\\\" rules_fired=\\\"%\" PRIuMAX \"\\\" "
            << "queue_size=\\\"%zu\\\"/>\\n\", size, gettime(), rules_fired_local, queue_size);\n"
          << "              } else {\n"
          << "                printf(\"\\t %zu states explored in %llus, with %\" PRIuMAX \" rules \"\n"
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
          << "            /* Avoid freeing the state we just added to the set. */\n"
          << "            n = NULL;\n"
          << "          }\n"
          << "        }\n"
          << "        free(n);\n"
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
  mpz_class offset = 0;
  for (const Ptr<Decl> &d : m.decls) {
    if (auto v = dynamic_cast<const VarDecl*>(d.get())) {
      generate_print(out, *v, "", offset);
      offset += v->width();
    }
  }
  out
    << "}\n\n";

  // Write a function to print state transitions.
  out
    << "static void print_transition(const struct state *s __attribute__((unused))) {\n"
    << "  ASSERT(s != NULL);\n"
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

    size_t index = 0;
    for (const Ptr<Rule> &r : flat_rules) {
      if (isa<StartState>(r)) {

        // Set up quantifiers.
        out << "    {\n";
        for (const Quantifier &q : r->quantifiers)
          generate_quantifier_header(out, q);

        out << "      rule_taken++;\n";

        // Close the quantifier loops.
        for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++)
          generate_quantifier_footer(out, *it);
        out << "    }\n";

        out
          << "  if (s->rule_taken <= rule_taken) {\n"
          << "    if (MACHINE_READABLE_OUTPUT) {\n"
          << "      printf(\"<transition>\");\n"
          << "      char *escaped_name = xml_escape(\""
            << (r->name == "" ? "Startstate " + std::to_string(index) : r->name)
            << "\");\n"
          << "      printf(\"%s\", escaped_name);\n"
          << "      free(escaped_name);\n"
          << "      printf(\"</transition>\\n\");\n"
          << "    } else {\n"
          << "      printf(\"Startstate %s fired.\\n\", \""
            << (r->name == "" ? "Startstate " + std::to_string(index) : r->name)
            << "\");\n"
          << "    }\n"
          << "    return;\n"
          << "  }\n";

        index++;
      }
    }
  }

  {
    out
      << "  } else {\n"
      << "    uint64_t rule_taken = 1;\n";

    size_t index = 0;
    for (const Ptr<Rule> &r : flat_rules) {
      if (isa<SimpleRule>(r)) {

        // Set up quantifiers.
        out << "    {\n";
        for (const Quantifier &q : r->quantifiers)
          generate_quantifier_header(out, q);

        out << "      rule_taken++;\n";

        // Close the quantifier loops.
        for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++)
          generate_quantifier_footer(out, *it);
        out << "    }\n";

        out
          << "  if (s->rule_taken <= rule_taken) {\n"
          << "    if (MACHINE_READABLE_OUTPUT) {\n"
          << "      printf(\"<transition>\");\n"
          << "      char *escaped_name = xml_escape(\""
            << (r->name == "" ? "Rule " + std::to_string(index) : r->name)
            << "\");\n"
          << "      printf(\"%s\", escaped_name);\n"
          << "      free(escaped_name);\n"
          << "      printf(\"</transition>\\n\");\n"
          << "    } else {\n"
          << "      printf(\"Rule %s fired.\\n\", \""
            << (r->name == "" ? "Rule " + std::to_string(index) : r->name)
            << "\");\n"
          << "    }\n"
          << "    return;\n"
          << "  }\n";

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
