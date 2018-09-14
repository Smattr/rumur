#include "generate.h"
#include <iostream>
#include <memory>
#include <rumur/rumur.h>

void generate_function(std::ostream &out, const rumur::Function &f) {

  /* Functions returning a simple type return a value, as expected. Functions
   * returning a complex type return a handle that is actually the same as their
   * second parameter (see below).
   */
  out << "static ";
  if (f.return_type != nullptr && f.return_type->is_simple()) {
    out << "value_t";
  } else {
    out << "struct handle";
  }
  out << " ru_" << f.name << "(struct state *s __attribute__((unused))";

  // If required, generate the return (out) parameter.
  if (f.return_type != nullptr && !f.return_type->is_simple())
    out << ", struct handle ret";

  for (const std::shared_ptr<rumur::Parameter> &p : f.parameters) {
    if (!p->by_reference && p->decl->type->is_simple()) {
      out << ", value_t ru_" << p->decl->name;
    } else {
      out << ", struct handle ru_" << p->decl->name;
    }
  }

  // TODO: decls

  out << ") {\n";

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

  out << "}";
}
