#include "../../common/escape.h"
#include "../../common/isa.h"
#include "generate.h"
#include "symmetry-reduction.h"
#include <cassert>
#include <cstddef>
#include <gmpxx.h>
#include <iostream>
#include <memory>
#include <rumur/rumur.h>
#include <string>
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

  // index counters for various things
  size_t start_index = 0;    // for start states
  size_t property_index = 0; // for property rules
  size_t rule_index = 0;     // for simple rules

  for (const Ptr<Node> &child : m.children) {

    // if this is a constant, emit it
    if (auto d = dynamic_cast<const ConstDecl *>(child.get())) {
      generate_decl(out, *d);
      out << ";\n\n";
      continue;
    }

    if (auto f = dynamic_cast<const Function *>(child.get())) {

      // create a list of the global declarations that are in scope (seen
      // previously) for this function
      std::vector<const Decl *> decls;
      for (const Ptr<Node> &c : m.children) {
        if (child.get() == c.get())
          break;
        if (auto d = dynamic_cast<const Decl *>(c.get()))
          decls.push_back(d);
      }

      generate_function(out, *f, decls);
      out << "\n\n";
      continue;
    }

    if (auto rule = dynamic_cast<const Rule *>(child.get())) {

      // Generate a set of flattened (non-hierarchical) rules. The purpose of
      // this is to essentially remove rulesets from the cases we need to deal
      // with below.
      const std::vector<Ptr<Rule>> rs = rule->flatten();

      for (const Ptr<Rule> &r : rs) {

        if (auto s = dynamic_cast<const StartState *>(r.get())) {
          out << "static bool startstate" << start_index
              << "(struct state *NONNULL s";
          for (const Quantifier &q : s->quantifiers)
            out << ", struct handle ru_" << q.name;
          out << ") {\n";

          out << "  static const char *rule_name __attribute__((unused)) = "
              << "\"startstate " << rule_name_string(*s, start_index)
              << "\";\n";

          out << "  if (JMP_BUF_NEEDED) {\n"
              << "    if (sigsetjmp(checkpoint, 0)) {\n"
              << "      /* error triggered during this startstate */\n"
              << "      return false;\n"
              << "    }\n"
              << "  }\n";

          // output the state variable handles that are in scope so we can
          // reference them within this start state
          for (const Ptr<Node> &c : m.children) {
            if (child.get() == c.get())
              break;
            if (auto d = dynamic_cast<const VarDecl *>(c.get())) {
              out << "  ";
              generate_decl(out, *d);
              out << ";\n";
            }
          }

          // output alias definitions, opening a scope in advance to support
          // aliases that shadow state variables, parameters, or other aliases
          for (const Ptr<AliasDecl> &a : s->aliases) {
            out << "   {\n  ";
            generate_decl(out, *a);
            out << ";\n";
          }

          // open a scope to support local declarations can shadow the state
          // variables
          out << "  {\n";

          for (const Ptr<Decl> &d : s->decls) {
            if (auto v = dynamic_cast<const VarDecl *>(d.get())) {
              out << "    ";
              generate_decl(out, *v);
              out << ";\n";
            }
          }

          // allocate memory for any complex-returning functions we call
          generate_allocations(out, s->body);

          for (auto &st : s->body) {
            out << "    ";
            generate_stmt(out, *st);
            out << ";\n";
          }

          // close the scopes we created
          out << "  }\n"
              << std::string(s->aliases.size(), '}') << "\n"
              << "  return true;\n"
              << "}\n\n";
          ++start_index;
        }

        if (auto p = dynamic_cast<const PropertyRule *>(r.get())) {
          out << "static __attribute__((unused)) bool property"
              << property_index << "(const struct state *NONNULL s";
          for (const Quantifier &q : p->quantifiers)
            out << ", struct handle ru_" << q.name;
          out << ") {\n";

          out << "  static const char *rule_name __attribute__((unused)) = "
                 "\"property "
              << rule_name_string(*p, property_index) << "\";\n";

          // output the state variable handles that are in scope so we can
          // reference them within this property
          for (const Ptr<Node> &c : m.children) {
            if (child.get() == c.get())
              break;
            if (auto d = dynamic_cast<const VarDecl *>(c.get())) {
              out << "  ";
              generate_decl(out, *d);
              out << ";\n";
            }
          }

          // output alias definitions, opening a scope in advance to support
          // aliases that shadow state variables, parameters, or other aliases
          for (const Ptr<AliasDecl> &a : p->aliases) {
            out << "   {\n  ";
            generate_decl(out, *a);
            out << ";\n";
          }

          out << "  return ";
          generate_property(out, p->property);
          out << ";\n"
              << std::string(p->aliases.size(), '}') << "\n"
              << "}\n\n";
          ++property_index;
        }

        if (auto s = dynamic_cast<const SimpleRule *>(r.get())) {

          // write the guard
          out << "static int guard" << rule_index
              << "(const struct state *NONNULL s __attribute__((unused))";
          for (const Quantifier &q : s->quantifiers)
            out << ", struct handle ru_" << q.name
                << " __attribute__((unused))";
          out << ") {\n";

          out << "  static const char *rule_name __attribute__((unused)) = \""
              << "guard of rule " << rule_name_string(*s, rule_index)
              << "\";\n";

          out << "  if (JMP_BUF_NEEDED) {\n"
              << "    if (sigsetjmp(checkpoint, 0)) {\n"
              << "      /* this guard triggered an error */\n"
              << "      return -1;\n"
              << "    }\n"
              << "  }\n";

          // output the state variable handles that are in scope so we can
          // reference them within this guard
          for (const Ptr<Node> &c : m.children) {
            if (child.get() == c.get())
              break;
            if (auto d = dynamic_cast<const VarDecl *>(c.get())) {
              out << "  ";
              generate_decl(out, *d);
              out << ";\n";
            }
          }

          // output alias definitions, opening a scope in advance to support
          // aliases that shadow state variables, parameters, or other aliases
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

          // write the body
          out << "static bool rule" << rule_index << "(struct state *NONNULL s";
          for (const Quantifier &q : s->quantifiers)
            out << ", struct handle ru_" << q.name;
          out << ") {\n";

          out << "  static const char *rule_name __attribute__((unused)) = "
              << "\"rule " << rule_name_string(*s, rule_index) << "\";\n";

          out << "  if (JMP_BUF_NEEDED) {\n"
              << "    if (sigsetjmp(checkpoint, 0)) {\n"
              << "      /* an error was triggered during this rule */\n"
              << "      return false;\n"
              << "    }\n"
              << "  }\n";

          // output the state variable handles that are in scope so we can
          // reference them within this rule
          for (const Ptr<Node> &c : m.children) {
            if (child.get() == c.get())
              break;
            if (auto d = dynamic_cast<const VarDecl *>(c.get())) {
              out << "  ";
              generate_decl(out, *d);
              out << ";\n";
            }
          }

          // output alias definitions, opening a scope in advance to support
          // aliases that shadow state variables, parameters, or other aliases
          for (const Ptr<AliasDecl> &a : s->aliases) {
            out << "   {\n  ";
            generate_decl(out, *a);
            out << ";\n";
          }

          // open a scope to support local declarations can shadow the state
          // variables
          out << "  {\n";

          for (const Ptr<Decl> &d : s->decls) {
            if (isa<VarDecl>(d)) {
              out << "  ";
              generate_decl(out, *d);
              out << ";\n";
            }
          }

          // allocate memory for any complex-returning functions we call
          generate_allocations(out, s->body);

          for (auto &st : s->body) {
            out << "  ";
            generate_stmt(out, *st);
            out << ";\n";
          }

          // Close the scopes we created.
          out << "  }\n"
              << std::string(s->aliases.size(), '}') << "\n"
              << "  return true;\n"
              << "}\n\n";

          rule_index++;
        }
      }
    }
  }

  // Write invariant checker
  {
    out << "static bool check_invariants(const struct state *NONNULL s "
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
    for (const Ptr<Node> &c : m.children) {
      if (auto rule = dynamic_cast<const Rule *>(c.get())) {

        // as above, we flatten the rule to avoid dealing with rulesets
        const std::vector<Ptr<Rule>> rs = rule->flatten();

        for (const Ptr<Rule> &r : rs) {
          if (auto p = dynamic_cast<const PropertyRule *>(r.get())) {
            if (p->property.category == Property::ASSERTION) {

              // open a scope so we do not have to think about name collisions
              out << "  {\n";

              // set up quantifiers
              for (const Quantifier &q : r->quantifiers)
                generate_quantifier_header(out, q);

              assert(index < property_index &&
                     "miscounted property rules during model generation");

              out << "    if (!property" << index << "(s";
              for (const Quantifier &q : r->quantifiers)
                out << ", ru_" << q.name;
              out << ")) {\n"
                  << "      error(s, \"invariant %s failed\", \""
                  << rule_name_string(*p, invariant_index) << "\");\n"
                  << "    }\n";

              // close the quantifier loops
              for (auto it = r->quantifiers.rbegin();
                   it != r->quantifiers.rend(); it++)
                generate_quantifier_footer(out, *it);

              // close this invariant's scope
              out << "  }\n";

              assert(invariant_index <= index &&
                     "incorrect invariant checker generation logic");
              ++invariant_index;
            }
            ++index;
          }
        }
      }
    }
    out << "  return true;\n"
        << "}\n\n";
  }

  // Write assumption checker
  {
    out << "static bool check_assumptions(const struct state *NONNULL s "
        << "__attribute__((unused))) {\n"
        << "  static const char *rule_name __attribute__((unused)) = NULL;\n"
        << "  if (JMP_BUF_NEEDED) {\n"
        << "    if (sigsetjmp(checkpoint, 0)) {\n"
        << "      /* one of the properties triggered an error */\n"
        << "      return false;\n"
        << "    }\n"
        << "  }\n";
    size_t index = 0;
    for (const Ptr<Node> &c : m.children) {
      if (auto rule = dynamic_cast<const Rule *>(c.get())) {

        // as above, we flatten the rule to avoid dealing with rulesets
        const std::vector<Ptr<Rule>> rs = rule->flatten();

        for (const Ptr<Rule> &r : rs) {
          if (auto p = dynamic_cast<const PropertyRule *>(r.get())) {
            if (p->property.category == Property::ASSUMPTION) {

              // open a scope so we do not have to think about name collisions
              out << "  {\n";

              // set up quantifiers
              for (const Quantifier &q : r->quantifiers)
                generate_quantifier_header(out, q);

              assert(index < property_index &&
                     "miscounted property rules during model generation");

              out << "    if (!property" << index << "(s";
              for (const Quantifier &q : r->quantifiers)
                out << ", ru_" << q.name;
              out << ")) {\n"
                  << "      /* Assumption violated. */\n"
                  << "      return false;\n"
                  << "    }\n";

              // close the quantifier loops
              for (auto it = r->quantifiers.rbegin();
                   it != r->quantifiers.rend(); it++)
                generate_quantifier_footer(out, *it);

              // close this assumptions's scope.
              out << "  }\n";
            }
            ++index;
          }
        }
      }
    }
    out << "  return true;\n"
        << "}\n\n";
  }

  // Write cover checker
  {
    out << "static bool check_covers(const struct state *NONNULL s "
        << "__attribute__((unused))) {\n"
        << "  static const char *rule_name __attribute__((unused)) = NULL;\n"
        << "  if (JMP_BUF_NEEDED) {\n"
        << "    if (sigsetjmp(checkpoint, 0)) {\n"
        << "      /* one of the properties triggered an error */\n"
        << "      return false;\n"
        << "    }\n"
        << "  }\n";
    size_t index = 0;
    for (const Ptr<Node> &c : m.children) {
      if (auto rule = dynamic_cast<const Rule *>(c.get())) {

        // as above, we flatten the rule to avoid dealing with rulesets
        const std::vector<Ptr<Rule>> rs = rule->flatten();

        for (const Ptr<Rule> &r : rs) {
          if (auto p = dynamic_cast<const PropertyRule *>(r.get())) {
            if (p->property.category == Property::COVER) {

              // open a scope so we do not have to think about name collisions
              out << "  {\n";

              // set up quantifiers
              for (const Quantifier &q : r->quantifiers)
                generate_quantifier_header(out, q);

              assert(index < property_index &&
                     "miscounted property rules during model generation");

              out << "    if (property" << index << "(s";
              for (const Quantifier &q : r->quantifiers)
                out << ", ru_" << q.name;
              out << ")) {\n"
                  << "      /* Covered. */\n"
                  << "      (void)__atomic_fetch_add(&covers[COVER_"
                  << p->property.unique_id << "], 1, __ATOMIC_SEQ_CST);\n"
                  << "    }\n";

              // close the quantifier loops
              for (auto it = r->quantifiers.rbegin();
                   it != r->quantifiers.rend(); it++)
                generate_quantifier_footer(out, *it);

              // close this cover's scope
              out << "  }\n";
            }
            ++index;
          }
        }
      }
    }
    out << "  return true;\n"
        << "}\n\n";
  }

  // Write liveness checker
  {
    out << "#if LIVENESS_COUNT > 0\n"
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
    for (const Ptr<Node> &c : m.children) {
      if (auto rule = dynamic_cast<const Rule *>(c.get())) {

        // as above, we flatten the rule to avoid dealing with rulesets
        const std::vector<Ptr<Rule>> rs = rule->flatten();

        for (const Ptr<Rule> &r : rs) {
          if (auto p = dynamic_cast<const PropertyRule *>(r.get())) {
            if (p->property.category == Property::LIVENESS) {

              // open a scope so we do not have to think about name collisions
              out << "  {\n";

              // set up quantifiers
              for (const Quantifier &q : r->quantifiers)
                generate_quantifier_header(out, q);

              assert(index < property_index &&
                     "miscounted property rules during model generation");

              out << "    if (property" << index << "(s";
              for (const Quantifier &q : r->quantifiers)
                out << ", ru_" << q.name;
              out << ")) {\n"
                  << "      /* Hit. */\n"
                  << "      mark_liveness(s, liveness_index, false);\n"
                  << "    }\n"
                  << "    liveness_index++;\n";

              // close the quantifier loops
              for (auto it = r->quantifiers.rbegin();
                   it != r->quantifiers.rend(); it++)
                generate_quantifier_footer(out, *it);

              // close this liveness property's scope
              out << "  }\n";
            }
            ++index;
          }
        }
      }
    }
    out << "  return true;\n"
        << "}\n\n";
  }

  // Write final liveness checker, the one that runs just prior to termination
  {
    out << "static void check_liveness_final(void) {\n"
        << "\n"
        << "  static const char *rule_name __attribute__((unused)) = NULL;\n"
        << "\n"
        << "  if (!MACHINE_READABLE_OUTPUT) {\n"
        << "    put(\"trying to prove remaining liveness "
           "constraints...\\n\");\n"
        << "  }\n"
        << "\n"
        << "  /* find how many liveness bits are unknown */\n"
        << "  unsigned long remaining = 0;\n"
        << "  unsigned long long last_update = 0;\n"
        << "  unsigned long learned_since_last = 0;\n"
        << "  if (!MACHINE_READABLE_OUTPUT) {\n"
        << "    for (size_t i = 0; i < set_size(local_seen); i++) {\n"
        << "\n"
        << "      slot_t slot = __atomic_load_n(&local_seen->bucket[i], "
           "__ATOMIC_SEQ_CST);\n"
        << "\n"
        << "      ASSERT(!slot_is_tombstone(slot)\n"
        << "        && \"seen set being migrated during final liveness "
           "check\");\n"
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
        << "    put(\"\\t \");\n"
        << "    put_uint(remaining);\n"
        << "    put(\" constraints remaining\\n\");\n"
        << "    last_update = gettime();\n"
        << "  }\n"
        << "\n"
        << "  bool progress = true;\n"
        << "  while (progress) {\n"
        << "    progress = false;\n"
        << "\n"
        << "    /* Run through all seen states trying to learn new liveness "
           "information. */\n"
        << "    for (size_t i = 0; i < set_size(local_seen); i++) {\n"
        << "\n"
        << "      slot_t slot = __atomic_load_n(&local_seen->bucket[i], "
           "__ATOMIC_SEQ_CST);\n"
        << "\n"
        << "      ASSERT(!slot_is_tombstone(slot)\n"
        << "        && \"seen set being migrated during final liveness "
           "check\");\n"
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
        << "        /* skip entries where liveness is fully satisfied already "
           "*/\n"
        << "        continue;\n"
        << "      }\n"
        << "\n"
        << "#if BOUND > 0\n"
        << "      /* If we're doing bounded checking and this state is at the "
           "bound limit,\n"
        << "       * it's not valid to expand beyond this.\n"
        << "       */\n"
        << "      ASSERT(state_bound_get(s) <= BOUND && \"a state that "
           "exceeded the bound depth was explored\");\n"
        << "      if (state_bound_get(s) == BOUND) {\n"
        << "        continue;\n"
        << "      }\n"
        << "#endif\n"
        << "\n";
    size_t index = 0;
    for (const Ptr<Node> &c : m.children) {
      if (auto rule = dynamic_cast<const Rule *>(c.get())) {
        const std::vector<Ptr<Rule>> rs = rule->flatten();
        for (const Ptr<Rule> &r : rs) {
          if (isa<SimpleRule>(r)) {

            assert(index < rule_index &&
                   "miscounted simple rules during model generation");

            // open a scope so we do not have to think about name collisions
            out << "      {\n";

            for (const Quantifier &q : r->quantifiers)
              generate_quantifier_header(out, q);

            out
                // use a dummy do-while to give us 'break' as a local goto
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
                << "            /* note that we can skip an invariant check "
                   "because we already know it\n"
                << "             * passed from prior expansion of this state.\n"
                << "             */\n"
                << "\n"
                << "            /* We should be able to find this state in the "
                   "seen set. */\n"
                << "            const struct state *t = set_find(n);\n"
                << "            ASSERT(t != NULL && \"state encountered during "
                   "final liveness wrap up \"\n"
                << "              \"that was not previously seen\");\n"
                << "\n"
                << "            /* See if this successor state learned a "
                   "liveness property it never\n"
                << "             * passed back to us. This can occur if the "
                   "state our exploration\n"
                << "             * encountered (`n`) was not the first of its "
                   "kind seen and thus was\n"
                << "             * de-duped and never made it into the seen "
                   "set with a back pointer\n"
                << "             * to `s`.\n"
                << "             */\n"
                << "            unsigned long learned = learn_liveness(s, t);\n"
                << "            if (learned > 0) {\n"
                << "              if (!MACHINE_READABLE_OUTPUT) {\n"
                << "                learned_since_last += learned;\n"
                << "                remaining -= learned;\n"
                << "                unsigned long long t = gettime();\n"
                << "                if (t > last_update) {\n"
                << "                  put(\"\\t \");\n"
                << "                  put_uint(learned_since_last);\n"
                << "                  put(\" further liveness constraints "
                   "proved in \");\n"
                << "                  put_uint(t - last_update);\n"
                << "                  put(\"s, with \");\n"
                << "                  put(green()); put_uint(remaining); "
                   "put(reset());\n"
                << "                  put(\" remaining\\n\");\n"
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

            // close the quantifier loops
            for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend();
                 it++)
              generate_quantifier_footer(out, *it);

            // close this rule's scope
            out << "}\n";

            ++index;
          }
        }
      }
    }
    out << "    }\n"
        << "  }\n"
        << "}\n"
        << "\n"
        << "\n"
        << "static unsigned long check_liveness_summarise(void) {\n"
        << "\n"
        << "  /* We can now finally check whether all liveness properties were "
           "hit. */\n"
        << "  bool missed[LIVENESS_COUNT];\n"
        << "  memset(missed, 0, sizeof(missed));\n"
        << "  for (size_t i = 0; i < set_size(local_seen); i++) {\n"
        << "\n"
        << "    slot_t slot = __atomic_load_n(&local_seen->bucket[i], "
           "__ATOMIC_SEQ_CST);\n"
        << "\n"
        << "    ASSERT(!slot_is_tombstone(slot)\n"
        << "      && \"seen set being migrated during final liveness "
           "check\");\n"
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
    for (const Ptr<Node> &c : m.children) {
      if (auto rule = dynamic_cast<const Rule *>(c.get())) {
        const std::vector<Ptr<Rule>> rs = rule->flatten();
        for (const Ptr<Rule> &r : rs) {
          if (auto p = dynamic_cast<const PropertyRule *>(r.get())) {
            if (p->property.category == Property::LIVENESS) {

              assert(index < property_index &&
                     "miscounted liveness properties during model generation");

              // open a scope so we don't have to think about name collisions
              out << "    {\n";

              // Set up quantifiers. Note, in this case they are not used.
              for (const Quantifier &q : r->quantifiers)
                generate_quantifier_header(out, q);

              out << "      size_t word_index = index / "
                     "(sizeof(s->liveness[0]) * CHAR_BIT);\n"
                  << "      size_t bit_index = index % (sizeof(s->liveness[0]) "
                     "* CHAR_BIT);\n"
                  << "      if (!missed[index] && !((s->liveness[word_index] "
                     ">> bit_index) & 0x1)) {\n"
                  << "        /* missed */\n"
                  << "        missed[index] = true;\n"
                  << "        if (MACHINE_READABLE_OUTPUT) {\n"
                  << "          put(\"<error includes_trace=\\\"\");\n"
                  << "          put(COUNTEREXAMPLE_TRACE == CEX_OFF ? "
                     "\"false\" : \"true\");\n"
                  << "          put(\"\\\">\\n<message>liveness property \");\n"
                  << "          xml_printf(\"" << rule_name_string(*p, index)
                  << "\");\n"
                  << "          put(\" violated</message>\\n\");\n"
                  << "        } else {\n"
                  << "          put(\"\\t\");\n"
                  << "          put(red()); put(bold());\n"
                  << "          put(\"liveness property "
                  << rule_name_string(*p, index) << " violated:\");\n"
                  << "          put(reset()); put(\"\\n\");\n"
                  << "        }\n"
                  << "        print_counterexample(s);\n"
                  << "        if (MACHINE_READABLE_OUTPUT) {\n"
                  << "          put(\"</error>\\n\");\n"
                  << "        }\n"
                  << "      }\n"
                  << "      index++;\n";

              // close the quantifier loops
              for (auto it = r->quantifiers.rbegin();
                   it != r->quantifiers.rend(); it++)
                generate_quantifier_footer(out, *it);

              // close this liveness property's scope
              out << "    }\n";

              ++index;
            }
          }
        }
      }
    }
    out << "  }\n"
        << "\n"
        << "  /* total up how many misses we saw */\n"
        << "  unsigned long total = 0;\n"
        << "  for (size_t i = 0; i < sizeof(missed) / sizeof(missed[0]); i++) "
           "{\n"
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
    out << "static void init(void) {\n"
        << "  static const char *rule_name __attribute__((unused)) = NULL;\n"
        << "  size_t queue_id = 0;\n"
        << "  uint64_t rule_taken = 1;\n";

    size_t index = 0;
    for (const Ptr<Node> &c : m.children) {
      if (auto rule = dynamic_cast<const Rule *>(c.get())) {
        const std::vector<Ptr<Rule>> rs = rule->flatten();
        for (const Ptr<Rule> &r : rs) {
          if (isa<StartState>(r)) {

            assert(index < start_index &&
                   "miscounted start states during model generation");

            // open a scope so we do not have to think about name collisions
            out << "  {\n";

            //  Define the state variable because the code emitted for
            // quantifiers expects it. They do not need a non-NULL value.
            out << "    struct state *s = NULL;\n";

            // set up quantifiers
            for (const Quantifier &q : r->quantifiers)
              generate_quantifier_header(out, q);

            out
                // use a dummy do-while to give us 'break' as a local goto
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
                << "          /* one of the cover properties triggered an "
                   "error */\n"
                << "          break;\n"
                << "        }\n"
                << "#if LIVENESS_COUNT > 0\n"
                << "        if (!check_liveness(s)) {\n"
                << "          /* one of the liveness properties triggered an "
                   "error */\n"
                << "          break;\n"
                << "        }\n"
                << "#endif\n"
                << "        (void)queue_enqueue(s, queue_id);\n"
                << "        queue_id = (queue_id + 1) % (sizeof(q) / "
                   "sizeof(q[0]));\n"
                << "      } else {\n"
                << "        state_free(s);\n"
                << "      }\n"
                << "    } while (0);\n"
                << "    rule_taken++;\n";

            // close the quantifier loops
            for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend();
                 it++)
              generate_quantifier_footer(out, *it);

            // close this startstate's scope
            out << "  }\n";

            ++index;
          }
        }
      }
    }
    out << "}\n\n";
  }

  // Write exploration logic
  {
    out << "static void explore(void) {\n"
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
    for (const Ptr<Node> &c : m.children) {
      if (auto rule = dynamic_cast<const Rule *>(c.get())) {
        const std::vector<Ptr<Rule>> rs = rule->flatten();
        for (const Ptr<Rule> &r : rs) {
          if (isa<SimpleRule>(r)) {

            assert(index < rule_index &&
                   "miscounted simple rules during model generation");

            // open a scope so we do not have to think about name collisions
            out << "    {\n";

            for (const Quantifier &q : r->quantifiers)
              generate_quantifier_header(out, q);

            out
                // use a dummy do-while to give us 'break' as a local goto
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
                << "          if (DEADLOCK_DETECTION != "
                   "DEADLOCK_DETECTION_STUTTERING || !state_eq(s, n)) {\n"
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
                << "              /* one of the cover properties triggered an "
                   "error */\n"
                << "              break;\n"
                << "            }\n"
                << "#if LIVENESS_COUNT > 0\n"
                << "            if (!check_liveness(n)) {\n"
                << "              /* one of the liveness properties triggered "
                   "an error */\n"
                << "              break;\n"
                << "            }\n"
                << "#endif\n"
                << "\n"
                << "#if BOUND > 0\n"
                << "            if (state_bound_get(n) < BOUND) {\n"
                << "#endif\n"
                << "            size_t queue_size = queue_enqueue(n, "
                   "thread_id);\n"
                << "            queue_id = thread_id;\n"
                << "\n"
                << "            if (size % 10000 == 0 && ftrylockfile(stdout) "
                   "== 0) {\n"
                << "              if (MACHINE_READABLE_OUTPUT) {\n"
                << "                put(\"<progress states=\\\"\");\n"
                << "                put_uint(size);\n"
                << "                put(\"\\\" duration_seconds=\\\"\");\n"
                << "                put_uint(gettime());\n"
                << "                put(\"\\\" rules_fired=\\\"\");\n"
                << "                put_uint(rules_fired_local);\n"
                << "                put(\"\\\" queue_size=\\\"\");\n"
                << "                put_uint(queue_size);\n"
                << "                put(\"\\\" thread_id=\\\"\");\n"
                << "                put_uint(thread_id);\n"
                << "                put(\"\\\"/>\\n\");\n"
                << "              } else {\n"
                << "                put(\"\\t \");\n"
                << "                if (THREADS > 1) {\n"
                << "                  put(\"thread \");\n"
                << "                  put_uint(thread_id);\n"
                << "                  put(\": \");\n"
                << "                }\n"
                << "                put_uint(size);\n"
                << "                put(\" states explored in \");\n"
                << "                put_uint(gettime());\n"
                << "                put(\"s, with \");\n"
                << "                put_uint(rules_fired_local);\n"
                << "                put(\" rules fired and \");\n"
                << "                put(queue_size > last_queue_size ? "
                   "yellow() : green());\n"
                << "                put_uint(queue_size);\n"
                << "                put(reset());\n"
                << "                put(\" states in the queue.\\n\");\n"
                << "              }\n"
                << "              funlockfile(stdout);\n"
                << "              last_queue_size = queue_size;\n"
                << "            }\n"
                << "\n"
                << "            if (THREADS > 1 && thread_id == 0 && phase == "
                   "WARMUP && queue_size > 20) {\n"
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

            // close the quantifier loops
            for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend();
                 it++)
              generate_quantifier_footer(out, *it);

            // close this rule's scope
            out << "}\n";

            ++index;
          }
        }
      }
    }
    out << "    /* If we did not toggle 'possible_deadlock' off by this point, "
           "we\n"
        << "     * have a deadlock.\n"
        << "     */\n"
        << "    if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_OFF && "
           "possible_deadlock) {\n"
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
  for (const Ptr<Node> &c : m.children) {
    if (auto d = dynamic_cast<const VarDecl *>(c.get())) {
      out << "  ";
      generate_decl(out, *d);
      out << ";\n";
    }
  }
  for (const Ptr<Node> &c : m.children) {
    if (auto v = dynamic_cast<const VarDecl *>(c.get()))
      generate_print(out, *v->type, v->name, "ru_" + v->name, true, true);
  }
  out << "}\n\n";

  // Write a function to print state transitions.
  out << "static void print_transition(const struct state *NONNULL s "
      << "__attribute__((unused))) {\n"
      << "  ASSERT(s != NULL);\n"
      << "  static const char *rule_name __attribute__((unused)) = NULL;\n"
      << "#if COUNTEREXAMPLE_TRACE != CEX_OFF\n"
      << "\n"
      << "  ASSERT(state_rule_taken_get(s) != 0 && \"unknown state "
         "transition\");\n"
      << "\n";

  {
    out << "  if (state_previous_get(s) == NULL) {\n"
        << "    uint64_t rule_taken = 1;\n";

    mpz_class base = 1;

    size_t index = 0;
    for (const Ptr<Node> &c : m.children) {
      if (auto rule = dynamic_cast<const Rule *>(c.get())) {
        const std::vector<Ptr<Rule>> rs = rule->flatten();
        for (const Ptr<Rule> &r : rs) {
          if (isa<StartState>(r)) {

            assert(index < start_index &&
                   "miscounted start states during model generation");

            // set up quantifiers
            out << "    {\n";
            for (const Quantifier &q : r->quantifiers)
              generate_quantifier_header(out, q);

            out << "  if (state_rule_taken_get(s) == rule_taken) {\n"
                << "    if (MACHINE_READABLE_OUTPUT) {\n"
                << "      put(\"<transition>\");\n"
                << "      xml_printf(\"Startstate "
                << rule_name_string(*r, index) << "\");\n"
                << "    } else {\n"
                << "      put(\"Startstate " << rule_name_string(*r, index)
                << "\");\n"
                << "    }\n";
            {
              size_t i = 0;
              for (const Quantifier &q : r->quantifiers) {
                out << "    {\n"
                    << "      value_t v = (value_t)((rule_taken - " << base
                    << ") / (1";
                size_t j = r->quantifiers.size() - 1;
                for (auto it = r->quantifiers.rbegin();
                     it != r->quantifiers.rend(); it++) {
                  if (i == j)
                    break;
                  out << " * " << it->count();
                  j--;
                }
                out << ") % " << q.count() << ") + " << q.lower_bound() << ";\n"

                    << "      if (MACHINE_READABLE_OUTPUT) {\n"
                    << "        put(\"<parameter name=\\\"\");\n"
                    << "        xml_printf(\"" << q.name << "\");\n"
                    << "        put(\"\\\">\");\n"
                    << "      } else {\n"
                    << "        put(\", " << q.name << ": \");\n"
                    << "      }\n";

                const Ptr<TypeExpr> t = q.type->resolve();
                if (auto e = dynamic_cast<const Enum *>(t.get())) {
                  size_t member_index = 0;
                  for (const std::pair<std::string, location> &member :
                       e->members) {
                    out << "      ";
                    if (member_index > 0)
                      out << "else ";
                    out << "if (v == VALUE_C(" << member_index << ")) {\n"
                        << "        if (MACHINE_READABLE_OUTPUT) {\n"
                        << "          xml_printf(\"" << member.first << "\");\n"
                        << "        } else {\n"
                        << "          put(\"" << member.first << "\");\n"
                        << "        }\n"
                        << "      }\n";
                    member_index++;
                  }
                  out << "      else {\n"
                      << "        ASSERT(!\"illegal value for " << q.name
                      << "\");\n"
                      << "      }\n";
                } else if (isa<Scalarset>(t)) {

                  // figure out if this is a named scalarset (i.e. ony eligible
                  // for symmetry reduction)
                  auto id = dynamic_cast<const TypeExprID *>(q.type.get());

                  if (id != nullptr) {

                    // remove any levels of indirection (TypeExprIDs of
                    // TypeExprIDs)
                    while (auto inner = dynamic_cast<const TypeExprID *>(
                               id->referent->value.get()))
                      id = inner;

                    // We do not need to do any schedule reversal because this
                    // is a start state. I.e. the implicit schedule under which
                    // this parameter was chosen is the identity permutation.

                    // dump the symbolic value of this parameter
                    out << "        if (USE_SCALARSET_SCHEDULES) {\n"
                        << "          put(\"" << escape(id->name) << "_\");\n"
                        << "          put_val(v);\n"
                        << "        } else {\n"
                        << "          put_val(v);\n"
                        << "        }\n";

                  } else {
                    // this scalarset seems not eligible for symmetry reduction
                    // (declared inline rather than as a TypeDecl), so fall back
                    // on just printing its value
                    out << "      put_val(v);\n";
                  }

                } else {
                  out << "      put_val(v);\n";
                }

                out << "      if (MACHINE_READABLE_OUTPUT) {\n"
                    << "        put(\"</parameter>\");\n"
                    << "      }\n"
                    << "    }\n";
                i++;
              }
            }
            out << "    if (MACHINE_READABLE_OUTPUT) {\n"
                << "      put(\"</transition>\\n\");\n"
                << "    } else {\n"
                << "      put(\" fired.\\n\");\n"
                << "    }\n"
                << "    return;\n"
                << "  }\n";

            out << "      rule_taken++;\n";

            // close the quantifier loops
            for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend();
                 it++)
              generate_quantifier_footer(out, *it);
            out << "    }\n";

            // update base for future comparison against rule_taken
            mpz_class inc = 1;
            for (const Quantifier &q : r->quantifiers) {
              inc *= q.count();
            }
            base += inc;

            ++index;
          }
        }
      }
    }
  }

  {
    out << "  } else {\n"
        << "    uint64_t rule_taken = 1;\n";

    mpz_class base = 1;

    size_t index = 0;
    for (const Ptr<Node> &c : m.children) {
      if (auto rule = dynamic_cast<const Rule *>(c.get())) {
        const std::vector<Ptr<Rule>> rs = rule->flatten();
        for (const Ptr<Rule> &r : rs) {
          if (isa<SimpleRule>(r)) {

            assert(index < rule_index &&
                   "miscounted simple rules during model generation");

            // set up quantifiers
            out << "    {\n";
            for (const Quantifier &q : r->quantifiers)
              generate_quantifier_header(out, q);

            out << "  if (state_rule_taken_get(s) == rule_taken) {\n"
                << "    if (MACHINE_READABLE_OUTPUT) {\n"
                << "      put(\"<transition>\");\n"
                << "      xml_printf(\"Rule " << rule_name_string(*r, index)
                << "\");\n"
                << "    } else {\n"
                << "      put(\"Rule " << rule_name_string(*r, index)
                << "\");\n"
                << "    }\n";
            {
              size_t i = 0;
              for (const Quantifier &q : r->quantifiers) {
                out << "    {\n"
                    << "      value_t v = (value_t)((rule_taken - " << base
                    << ") / (1";
                size_t j = r->quantifiers.size() - 1;
                for (auto it = r->quantifiers.rbegin();
                     it != r->quantifiers.rend(); it++) {
                  if (i == j)
                    break;
                  out << " * " << it->count();
                  j--;
                }
                out << ") % " << q.count() << ") + " << q.lower_bound() << ";\n"

                    << "      if (MACHINE_READABLE_OUTPUT) {\n"
                    << "        put(\"<parameter name=\\\"\");\n"
                    << "        xml_printf(\"" << q.name << "\");\n"
                    << "        put(\"\\\">\");\n"
                    << "      } else {\n"
                    << "        put(\", " << q.name << ": \");\n"
                    << "      }\n";

                const Ptr<TypeExpr> t = q.type->resolve();
                if (auto e = dynamic_cast<const Enum *>(t.get())) {
                  size_t member_index = 0;
                  for (const std::pair<std::string, location> &member :
                       e->members) {
                    out << "      ";
                    if (member_index > 0)
                      out << "else ";
                    out << "if (v == VALUE_C(" << member_index << ")) {\n"
                        << "        if (MACHINE_READABLE_OUTPUT) {\n"
                        << "          xml_printf(\"" << member.first << "\");\n"
                        << "        } else {\n"
                        << "          put(\"" << member.first << "\");\n"
                        << "        }\n"
                        << "      }\n";
                    member_index++;
                  }
                  out << "      else {\n"
                      << "        ASSERT(!\"illegal value for " << q.name
                      << "\");\n"
                      << "      }\n";
                } else if (auto s = dynamic_cast<const Scalarset *>(t.get())) {

                  // open a scope to contain the schedule computation variables
                  out << "      {\n";

                  const std::string b = "((size_t)" +
                                        s->bound->constant_fold().get_str() +
                                        "ull)";

                  // figure out if this is a named scalarset (i.e. ony eligible
                  // for symmetry reduction)
                  auto id = dynamic_cast<const TypeExprID *>(q.type.get());

                  if (id != nullptr) {

                    // remove any levels of indirection (TypeExprIDs of
                    // TypeExprIDs)
                    while (auto inner = dynamic_cast<const TypeExprID *>(
                               id->referent->value.get()))
                      id = inner;

                    // generate schedule retrieval
                    out << "        size_t schedule[" << b << "];\n"
                        << "        /* setup a default identity mapping for "
                           "when\n"
                        << "         * symmetry reduction is off\n"
                        << "         */\n"
                        << "        for (size_t i = 0; i < " << b
                        << "; ++i) {\n"
                        << "          schedule[i] = i;\n"
                        << "        }\n"
                        << "        if (USE_SCALARSET_SCHEDULES) {\n"
                        // note that we read from the *previous* state’s
                        // schedule here because that is what this value is
                        // relative to
                        << "          size_t index = schedule_read_" << id->name
                        << "(state_previous_get(s));\n"
                        << "          size_t stack[" << b << "];\n"
                        << "          index_to_permutation(index, schedule, "
                           "stack, "
                        << b << ");\n"
                        << "        }\n";

                    // map the parameter value through the retrieved permutation
                    out << "        assert((size_t)v < " << b
                        << " && \"illegal scalarset "
                        << " parameter recorded\");\n"
                        << "        v = (value_t)schedule[(size_t)v];\n";

                    // dump the resulting value
                    out << "        if (USE_SCALARSET_SCHEDULES) {\n"
                        << "          put(\"" << escape(id->name) << "_\");\n"
                        << "          put_val(v);\n"
                        << "        } else {\n"
                        << "          put_val(v);\n"
                        << "        }\n";

                  } else {
                    // this scalarset seems not eligible for symmetry reduction
                    // (declared inline rather than as a TypeDecl), so fall back
                    // on just printing its value
                    out << "      put_val(v);\n";
                  }

                  out << "}\n";

                } else {
                  out << "      put_val(v);\n";
                }

                out << "      if (MACHINE_READABLE_OUTPUT) {\n"
                    << "        put(\"</parameter>\");\n"
                    << "      }\n"
                    << "    }\n";
                i++;
              }
            }
            out << "    if (MACHINE_READABLE_OUTPUT) {\n"
                << "      put(\"</transition>\\n\");\n"
                << "    } else {\n"
                << "      put(\" fired.\\n\");\n"
                << "    }\n"
                << "    return;\n"
                << "  }\n";

            out << "      rule_taken++;\n";

            // Close the quantifier loops.
            for (auto it = r->quantifiers.rbegin(); it != r->quantifiers.rend();
                 it++)
              generate_quantifier_footer(out, *it);
            out << "    }\n";

            // update base for future comparison against rule_taken
            mpz_class inc = 1;
            for (const Quantifier &q : r->quantifiers) {
              inc *= q.count();
            }
            base += inc;

            ++index;
          }
        }
      }
    }
  }

  out << "  }\n"
      << "\n"
      << "  /* give some helpful output for debugging problems with this "
         "function. */\n"
      << "  fprintf(stderr, \"no rule found to link to state at depth "
         "%zu\\n\", state_depth(s));\n"
      << "  ASSERT(!\"unreachable\");\n"
      << "#endif\n"
      << "}\n\n";

  // Generate a function used during debugging
  out << "static void state_print_field_offsets(void) {\n"
      << "  put(\"\t* state struct is \");\n"
      << "  put_uint(__alignof__(struct state));\n"
      << "  put(\"-byte aligned\\n\");\n";
  for (const Ptr<Node> &c : m.children) {
    if (auto v = dynamic_cast<const VarDecl *>(c.get()))
      out << "  put(\"\t* field " << v->name << " is located at state offset "
          << v->offset << " bits\\n\");\n";
  }
  out << "  put(\"\\n\");\n"
      << "}\n\n";
}
