#include "generate.h"
#include <iostream>
#include <memory>
#include <rumur/rumur.h>
#include "utils.h"
#include <vector>

void generate_function(std::ostream &out, const rumur::Function &f,
    const std::vector<std::shared_ptr<rumur::Decl>> &decls) {

  /* Functions returning a simple type return a value, as expected. Functions
   * returning a complex type return a handle that is actually the same as their
   * second parameter (see below).
   */
  out << "static ";
  if (f.return_type == nullptr) {
    out << "void";
  } else if (f.return_type->is_simple()) {
    out << "value_t";
  } else {
    out << "struct handle";
  }
  out << " ru_" << f.name << "(struct state *s __attribute__((unused))";

  // If required, generate the return (out) parameter.
  if (f.return_type != nullptr && !f.return_type->is_simple())
    out << ", struct handle ret";

  for (const std::shared_ptr<rumur::VarDecl> &p : f.parameters) {
    if (p->readonly && p->type->is_simple()) {
      out << ", value_t ru_" << p->name;
    } else {
      out << ", struct handle ru_" << p->name;
    }
  }

  out << ") {\n";

  /* Output the state variable handles so we can reference them within
   * this start state.
   */
  for (const std::shared_ptr<rumur::Decl> &d : decls) {
    if (isa<rumur::VarDecl>(d)) {

      /* Exciting kludge: we need to suppress the definition of state variables
       * that are shadowed by function parameters. Yes, real world models seem
       * to do this.
       */
      bool shadowed = false;
      for (const std::shared_ptr<rumur::VarDecl> &p : f.parameters) {
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
  for (const std::shared_ptr<rumur::Decl> &d : f.decls) {
    if (isa<rumur::ConstDecl>(d) || isa<rumur::VarDecl>(d)) {
      out << "  ";
      generate_decl(out, *d);
      out << ";\n";
    }
  }

  // Allocate memory for any complex-returning functions we call
  generate_allocations(out, f.body);

  // Generate the body of the function
  for (const std::shared_ptr<rumur::Stmt> &s : f.body) {
    out << "  ";
    generate_stmt(out, *s);
    out << ";\n";
  }

  /* Emit a guard against the user leaving a control flow path through their
   * function that doesn't return.
   */
  if (f.return_type != nullptr)
    out << "  error(s, false, \"The end of function %s reached without "
      << "returning values.\", \"" << f.name << "\");\n";

  // Close the scope we created.
  out << "  }\n";

  out << "}";
}
