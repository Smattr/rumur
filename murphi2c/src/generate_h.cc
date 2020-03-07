#include <cstddef>
#include "CodeGenerator.h"
#include "generate_c.h"
#include "generate_h.h"
#include <iostream>
#include "resources.h"
#include <rumur/rumur.h>

using namespace rumur;

namespace {

class HGenerator : public CodeGenerator, public ConstTraversal {

 private:
  std::ostream &out;

 public:
  HGenerator(std::ostream &out_, bool): out(out_) { }

  // helpers to make output below more natural

  HGenerator &operator<<(const std::string &s) {
    out << s;
    return *this;
  }

  HGenerator &operator<<(const Node &n) {
    dispatch(n);
    return *this;
  }

  void visit_constdecl(const ConstDecl &n) final {
    *this << indentation() << "extern const ";
    if (n.type == nullptr) {
      *this << "int64_t";
    } else {
      *this << *n.type;
    }
    *this << " " << n.name << ";\n";
  }

  void visit_function(const Function &n) final {
    *this << indentation();
    if (n.return_type == nullptr) {
      *this << "void";
    } else {
      *this << *n.return_type;
    }
    *this << " " << n.name << "(";
    bool first = true;
    for (const Ptr<VarDecl> &p : n.parameters) {
      if (!first) {
        *this << ", ";
      }
      *this << *p->type << " ";
      // if this is a var parameter, it needs to be a pointer
      if (!p->readonly) {
        *this << "*" << p->name << "_";
      } else {
        *this << p->name;
      }
      first = false;
    }
    *this << ");\n";
  }

  void visit_model(const Model &n) final {

    // constants, types and variables
    for (const Ptr<Decl> &d : n.decls) {
      *this << *d;
    }

    *this << "\n";

    // functions and procedures
    for (const Ptr<Function> &f : n.functions) {
      *this << *f << "\n";
    }

    // flatten the rules so we do not have to deal with the hierarchy of
    // rulesets, aliasrules, etc.
    std::vector<Ptr<Rule>> flattened;
    for (const Ptr<Rule> &r : n.rules) {
      std::vector<Ptr<Rule>> rs = r->flatten();
      flattened.insert(flattened.end(), rs.begin(), rs.end());
    }

    // startstates, rules, invariants
    for (const Ptr<Rule> &r : flattened) {
      *this << *r << "\n";
    }
  }

  void visit_vardecl(const VarDecl &n) final {
    *this << indentation() << "extern " << *n.type << " " << n.name << ";\n";
  }
};

}

void generate_h(const Node &n, bool pack, std::ostream &out) {

  // write the static prefix to the beginning of the source file
  for (size_t i = 0; i < resources_h_prefix_h_len; i++)
    out << (char)resources_h_prefix_h[i];

  HGenerator gen(out, pack);
  gen.dispatch(n);

  // close the `extern "C"` block opened in ../resources/h_prefix.h
  out
    << "\n"
    << "#ifdef __cplusplus\n"
    << "}\n"
    << "#endif\n";
}
