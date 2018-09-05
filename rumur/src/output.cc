#include <gmpxx.h>
#include <iostream>
#include <fstream>
#include <memory>
#include "options.h"
#include "output.h"
#include <rumur/rumur.h>
#include "resources.h"
#include <string>
#include "symmetry-reduction.h"
#include <utility>
#include <vector>

using namespace rumur;

static std::ostream &operator<<(std::ostream &out, const Model &m) {

  /* Generate a set of flattened (non-hierarchical) rules. The purpose of this
   * is to essentially remove rulesets from the cases we need to deal with
   * below.
   */
  std::vector<std::shared_ptr<Rule>> flat_rules;
  for (const std::shared_ptr<Rule> &r : m.rules) {
    std::vector<std::shared_ptr<Rule>> rs = r->flatten();
    flat_rules.insert(flat_rules.end(), rs.begin(), rs.end());
  }

  // Write out the start state rules.
  {
    size_t index = 0;
    for (const std::shared_ptr<Rule> &r : flat_rules) {
      if (auto s = dynamic_cast<const StartState*>(r.get())) {
        out << "static void startstate" << index << "(struct state *s";
        for (const std::shared_ptr<Quantifier> &q : s->quantifiers)
          out << ", struct handle ru_" << q->var->name;
        out << ") {\n";

        for (const std::shared_ptr<Decl> &d : s->decls) {
          if (auto v = dynamic_cast<const VarDecl*>(d.get()))
            out << "  uint8_t _ru_" << v->name << "[BITS_TO_BYTES("
              << v->type->width() << ")] = { 0 };\n"
              << "  struct handle ru_" << v->name << " = { .base = _ru_"
              << v->name << ", .offset = 0, .width = SIZE_C("
              << v->type->width() << ") };\n";
        }

        for (const std::shared_ptr<Stmt> &st : s->body)
          out << "  " << *st << ";\n";
        out << "}\n\n";
        index++;
      }
    }
  }

  // Write out the property rules.
  {
    size_t index = 0;
    for (const std::shared_ptr<Rule> &r : flat_rules) {
      if (auto i = dynamic_cast<const PropertyRule*>(r.get())) {
        out << "static __attribute__((unused)) bool property" << index << "(const struct state *s";
        for (const std::shared_ptr<Quantifier> &q : i->quantifiers)
          out << ", struct handle ru_" << q->var->name;
        out << ") {\n"
          << "  return ";
        i->property.expr->generate_rvalue(out);
        out << ";\n}\n\n";
        index++;
      }
    }
  }

  // Write out the regular rules.
  {
    size_t index = 0;
    for (const std::shared_ptr<Rule> &r : flat_rules) {
      if (auto s = dynamic_cast<const SimpleRule*>(r.get())) {

        // Write the guard
        out << "static bool guard" << index << "(const struct state *s "
          "__attribute__((unused))";
        for (const std::shared_ptr<Quantifier> &q : s->quantifiers)
          out << ", struct handle ru_" << q->var->name
            << " __attribute__((unused))";
        out << ") {\n  return ";
        if (s->guard == nullptr) {
          out << "true";
        } else {
          s->guard->generate_rvalue(out);
        }
        out << ";\n}\n\n";

        // Write the body
        out << "static void rule" << index << "(struct state *s";
        for (const std::shared_ptr<Quantifier> &q : s->quantifiers)
          out << ", struct handle ru_" << q->var->name;
        out << ") {\n";

        for (const std::shared_ptr<Decl> &d : s->decls) {
          if (auto v = dynamic_cast<const VarDecl*>(d.get()))
            out << "  uint8_t _ru_" << v->name << "[BITS_TO_BYTES("
              << v->type->width() << ")] = { 0 };\n"
              << "  struct handle ru_" << v->name << " = { .base = _ru_"
              << v->name << ", .offset = 0, .width = SIZE_C("
              << v->type->width() << ") };\n";
        }

        for (const std::shared_ptr<Stmt> &st : s->body)
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
    for (const std::shared_ptr<Rule> &r : flat_rules) {
      if (auto p = dynamic_cast<const PropertyRule*>(r.get())) {
        if (p->property.category == Property::ASSERTION) {

          // Open a scope so we don't have to think about name collisions.
          out << "  {\n";

          // Set up quantifiers.
          for (const std::shared_ptr<Quantifier> &q : r->quantifiers)
            q->generate_header(out);

          out << "    if (!property" << index << "(s";
          for (const std::shared_ptr<Quantifier> &q : r->quantifiers)
            out << ", ru_" << q->var->name;
          out << ")) {\n"
            << "      error(s, false, \"failed invariant\");\n"
            << "    }\n";

          // Close the quantifier loops.
          for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++)
            (*it)->generate_footer(out);

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
    for (const std::shared_ptr<Rule> &r : flat_rules) {
      if (auto p = dynamic_cast<const PropertyRule*>(r.get())) {
        if (p->property.category == Property::ASSUMPTION) {

          // Open a scope so we don't have to think about name collisions.
          out << "  {\n";

          // Set up quantifiers.
          for (const std::shared_ptr<Quantifier> &q : r->quantifiers)
            q->generate_header(out);

          out << "    if (!property" << index << "(s";
          for (const std::shared_ptr<Quantifier> &q : r->quantifiers)
            out << ", ru_" << q->var->name;
          out << ")) {\n"
            << "      /* Assumption violated. */\n"
            << "      assert(JMP_BUF_NEEDED && \"longjmping without a setup jmp_buf\");\n"
            << "      longjmp(checkpoint, 1);\n"
            << "    }\n";

          // Close the quantifier loops.
          for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++)
            (*it)->generate_footer(out);

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
      << "  size_t queue_id = 0;\n";

    size_t index = 0;
    for (const std::shared_ptr<Rule> &r : flat_rules) {
      if (dynamic_cast<const StartState*>(r.get()) != nullptr) {

        // Open a scope so we don't have to think about name collisions.
        out << "  {\n";

        /* Define the state variable because the code emitted for quantifiers
         * expects it. They do not need a non-NULL value.
         */
        out << "    struct state *s = NULL;\n";

        // Set up quantifiers.
        for (const std::shared_ptr<Quantifier> &q : r->quantifiers)
          q->generate_header(out);

        out
          // Use a dummy do-while to give us 'break' as a local goto.
          << "    do {\n"

          << "      s = state_new();\n"
          << "      if (JMP_BUF_NEEDED) {\n"
          << "        if (setjmp(checkpoint)) {\n"
          << "          /* error() was called. */\n"
          << "          break;\n"
          << "        }\n"
          << "      }\n"
          << "      startstate" << index << "(s";
        for (const std::shared_ptr<Quantifier> &q : r->quantifiers)
          out << ", ru_" << q->var->name;
        out << ");\n"
          << "      check_assumptions(s);\n"
          << "      check_invariants(s);\n"
          << "      if (SYMMETRY_REDUCTION) {\n"
          << "        state_canonicalise(s);\n"
          << "      }\n"
          << "      size_t size;\n"
          << "      if (set_insert(s, &size)) {\n"
          << "        (void)queue_enqueue(s, queue_id);\n"
          << "        queue_id = (queue_id + 1) % (sizeof(q) / sizeof(q[0]));\n"
          << "        s = NULL;\n"
          << "      }\n"
          << "      free(s);\n"
          << "    } while (0);\n";

        // Close the quantifier loops.
        for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++)
          (*it)->generate_footer(out);

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
      << "    bool possible_deadlock = true;\n";
    size_t index = 0;
    for (const std::shared_ptr<Rule> &r : flat_rules) {
      if (dynamic_cast<const SimpleRule*>(r.get()) != nullptr) {

        // Open a scope so we don't have to think about name collisions.
        out << "    {\n";

        for (const std::shared_ptr<Quantifier> &q : r->quantifiers)
          q->generate_header(out);

        out
          // Use a dummy do-while to give us 'break' as a local goto.
          << "      do {\n"
          << "        struct state *n = state_dup(s);\n"
          << "        if (JMP_BUF_NEEDED) {\n"
          << "          if (setjmp(checkpoint)) {\n"
          << "            /* error() was called. */\n"
          << "            break;\n"
          << "          }\n"
          << "        }\n"
          << "        if (guard" << index << "(n";
        for (const std::shared_ptr<Quantifier> &q : r->quantifiers)
          out << ", ru_" << q->var->name;
        out << ")) {\n"
          << "          possible_deadlock = false;\n"
          << "          rule" << index << "(n";
        for (const std::shared_ptr<Quantifier> &q : r->quantifiers)
          out << ", ru_" << q->var->name;
        out << ");\n"
          << "          rules_fired_local++;\n"
          << "          check_assumptions(s);\n"
          << "          check_invariants(n);\n"
          << "          if (SYMMETRY_REDUCTION) {\n"
          << "            state_canonicalise(n);\n"
          << "          }\n"
          << "          size_t size;\n"
          << "          if (set_insert(n, &size)) {\n"
          << "\n"
          << "            size_t queue_size = queue_enqueue(n, thread_id);\n"
          << "            queue_id = thread_id;\n"
          << "\n"
          << "            if (size % 10000 == 0) {\n"
          << "              print_lock();\n"
          << "              printf(\"\\t %zu states explored in %llus, with %\" PRIuMAX \" rules \"\n"
          << "                \"fired and %s%zu%s states in the queue.\\n\", size, gettime(),\n"
          << "                rules_fired_local, queue_size > last_queue_size ? yellow() : green(),\n"
          << "                queue_size, reset());\n"
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
          << "      } while (0);\n";

        // Close the quantifier loops.
        for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend(); it++)
          (*it)->generate_footer(out);

        // Close this rule's scope.
        out << "}\n";

        index++;
      }
    }
    out
      << "    /* If we did not toggle 'possible_deadlock' off by this point, we\n"
      << "     * have a deadlock.\n"
      << "     */\n"
      << "    if (DEADLOCK_DETECTION && possible_deadlock) {\n"
      << "      error(s, true, \"deadlock\");\n"
      << "    }\n"
      << "\n"
      << "  }\n"
      << "  exit_with(EXIT_SUCCESS);\n"
      << "}\n\n";
  }

  // Write a function to print the state.
  out
    << "static void state_print(const struct state *s) {\n";
  mpz_class offset = 0;
  for (const std::shared_ptr<Decl> &d : m.decls) {
    if (auto v = dynamic_cast<const VarDecl*>(d.get())) {
      v->generate_print(out, "", offset);
      offset += v->width();
    }
  }
  out
    << "}\n\n";

  return out;
}

int output_checker(const std::string &path, const Model &model) {

  std::ofstream out(path);
  if (!out)
    return -1;

  if (!options.debug)
    out << "#define NDEBUG 1\n\n";

  out

    // #includes
    << std::string((const char*)resources_includes_c, (size_t)resources_includes_c_len)
    << "\n"

    << "enum { SET_CAPACITY = " << options.set_capacity << "ul };\n\n"
    << "enum { SET_EXPAND_THRESHOLD = " << options.set_expand_threshold << " };\n\n"
    << "static const enum { OFF, ON, AUTO } COLOR = " << (options.color == OFF ? "OFF" :
      options.color == ON ? "ON" : "AUTO") << ";\n\n"
    << "enum trace_category_t {\n"
    << "  TC_HANDLE_READS       = " << TC_HANDLE_READS << ",\n"
    << "  TC_HANDLE_WRITES      = " << TC_HANDLE_WRITES << ",\n"
    << "  TC_QUEUE              = " << TC_QUEUE << ",\n"
    << "  TC_SET                = " << TC_SET << ",\n"
    << "  TC_SYMMETRY_REDUCTION = " << TC_SYMMETRY_REDUCTION << ",\n"
    << "};\n"
    << "static const uint64_t TRACES_ENABLED = UINT64_C(" << options.traces << ");\n\n"
    << "enum { DEADLOCK_DETECTION = " << options.deadlock_detection << " };\n\n"
    << "enum { SYMMETRY_REDUCTION = " << options.symmetry_reduction << " };\n\n"
    << "enum { SANDBOX_ENABLED = " << options.sandbox_enabled << " };\n\n"
    << "enum { MAX_ERRORS = " << options.max_errors << "ul };\n\n"

    // Settings that are used in header.c
    << "enum { THREADS = " << options.threads << "ul };\n\n"
    << "enum { STATE_SIZE_BITS = " << model.size_bits() << "ul };\n\n"
    << "enum { ASSUMPTION_COUNT = " << model.assumption_count() << "ul };\n\n"

    // Static boiler plate code
    << std::string((const char*)resources_header_c, (size_t)resources_header_c_len)
    << "\n"

    // the model itself
    << model;

  return 0;

}
