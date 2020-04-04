#include <cstddef>
#include <cassert>
#include <ctype.h>
#include <rumur/rumur.h>
#include "Stage.h"
#include <string>
#include "SwitchToIf.h"

using namespace rumur;

SwitchToIf::SwitchToIf(Stage &next_): IntermediateStage(next_) { }

void SwitchToIf::write(const std::string &c) {

  // if we already understand indentation, we can just pass through
  if (learned_indentation) {
    next.write(c);
    return;
  }

  // if this character is a newline, assume we have an opportunity to learn
  // indentation coming up
  if (c == "\n") {
    last_newline = true;
    indentation = "";
    next.write(c);
    return;
  }

  // if we have not just seen a newline, assume we are not currently writing
  // indentation
  if (!last_newline) {
    next.write(c);
    return;
  }

  // if this is white space, continue accruing
  if (c.size() == 1 && isspace(c[0])) {
    indentation += c;

  // otherwise, if we have found a non-zero offset, we know the indentation
  } else if (indentation != "") {
    learned_indentation = true;

  // otherwise, we are looking at a non-indented line
  } else {
    last_newline = false;

  }

  next.write(c);
}

void SwitchToIf::visit_switch(const Switch &n) {

  // we need to emit the switched-on expression multiple times, so we require it
  // to be side-effect free
  if (!n.expr->is_pure())
    throw Error("impure expression in switch statement expression prevents "
      "transforming this into an if-then-else", n.loc);

  // write everything up to the start of this expression
  top->sync_to(n);

  // handle an edge case where the switch has no cases and should be omitted
  // entirely
  if (n.cases.empty()) {
    top->skip_to(n.loc.end);
    return;
  }

  // see if we can figure out what level of indentation this switch statement is
  // at
  assert(n.loc.begin.column > 0);
  size_t spaces = static_cast<size_t>(n.loc.begin.column) - 1;
  std::string indent;
  if (learned_indentation && spaces % indentation.size() == 0) {
    for (size_t i = 0; i * indentation.size() < spaces; i++)
      indent += indentation;
  }

  std::string sep;
  for (const SwitchCase &c : n.cases) {

    // seek up to this case
    top->skip_to(c);

    if (c.matches.empty()) {
      *top << sep << "e\n";
    } else {
      *top << sep << "if ";

      std::string sep2;
      bool brackets = c.matches.size() > 1;
      for (const Ptr<Expr> &m : c.matches) {
        *top << sep2 << (brackets ? "(" : "") << n.expr->to_string() << " = ";
        top->skip_to(*m);
        top->dispatch(*m);
        *top << (brackets ? ")" : "");
        sep2 = " | ";
      }

      *top << " then\n";
    }

    if (!c.body.empty()) {
      top->skip_to(*c.body[0]);
      if (learned_indentation)
        *top << indent << indentation;
    }
    for (const Ptr<Stmt> &s : c.body)
      top->dispatch(*s);
    if (!c.body.empty())
      *top << ";";

    sep = "\n" + indent + "els";
  }

  *top << "\n" << indent << "endif";
  top->skip_to(n.loc.end);
}
