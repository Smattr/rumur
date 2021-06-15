#include "../../common/isa.h"
#include "generate.h"
#include <cstddef>
#include <iostream>
#include <memory>
#include <rumur/rumur.h>
#include <vector>

using namespace rumur;

void generate_function(std::ostream &out, const Function &f,
                       const std::vector<const Decl *> &decls) {

  out << "static ";

  bool needs_return_handle =
      f.return_type != nullptr && !f.return_type->is_simple();

  // if this function has no side effects and doesn’t have output arguments,
  // pass the compiler a hint for this
  if (!needs_return_handle && f.is_pure())
    out << "__attribute__((pure)) ";

  /* Functions returning a simple type return a value, as expected. Functions
   * returning a complex type return a handle that is actually the same as their
   * second parameter (see below).
   */
  if (f.return_type == nullptr) {
    /* We need to give void-returning functions a dummy boolean return type to
     * align with the type signature for rules. More specifically a return
     * can appear either within a rule or within a function/procedure. Within a
     * rule, it needs to return true to indicate to the caller no errors were
     * encountered during the rule. To allow a return statement to be emitted
     * uniformly without having to first check whether its within a rule or a
     * function/procedure we make the latter return a (ignored) boolean as well.
     */
    out << "bool";
  } else if (f.return_type->is_simple()) {
    out << "value_t";
  } else {
    out << "struct handle";
  }
  out << " ru_" << f.name << "(const char *rule_name __attribute__((unused)), "
      << "struct state *NONNULL s __attribute__((unused))";

  // If required, generate the return (out) parameter.
  if (needs_return_handle)
    out << ", struct handle ret";

  for (const Ptr<VarDecl> &p : f.parameters)
    out << ", struct handle ru_" << p->name;

  out << ") {\n";

  /* Output the state variable handles so we can reference them within
   * this start state.
   */
  for (const Decl *d : decls) {
    if (isa<VarDecl>(d)) {

      /* Exciting kludge: we need to suppress the definition of state variables
       * that are shadowed by function parameters. Yes, real world models seem
       * to do this.
       */
      bool shadowed = false;
      for (const Ptr<VarDecl> &p : f.parameters) {
        if (p->name == d->name) {
          shadowed = true;
          break;
        }
      }
      if (shadowed)
        continue;

      out << "  ";
      generate_decl(out, *d);
      out << ";\n";
    }
  }

  /* Open a scope to support local declarations can shadow the state
   * variables.
   */
  out << "  {\n";

  // Output this function's local decls
  for (const Ptr<Decl> &d : f.decls) {
    if (isa<ConstDecl>(d) || isa<VarDecl>(d)) {
      out << "  ";
      generate_decl(out, *d);
      out << ";\n";
    }
  }

  // Allocate memory for any complex-returning functions we call
  generate_allocations(out, f.body);

  // Generate the body of the function
  for (auto &s : f.body) {
    out << "  ";
    generate_stmt(out, *s);
    out << ";\n";
  }

  /* Emit a guard against the user leaving a control flow path through their
   * function that doesn't return.
   */
  if (f.return_type != nullptr)
    out << "  error(s, \"The end of function %s reached without returning "
        << "values.\", \"" << f.name << "\");\n";

  if (f.return_type == nullptr)
    out << "  return true; /* ignored by caller */\n";

  // Close the scope we created.
  out << "  }\n";

  out << "}";
}
