#include <iostream>
#include "location.hh"
#include <memory>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/Node.h>
#include <string>

using namespace rumur;
using namespace std;

Decl::Decl(const string &name, const location &loc)
  : Node(loc), name(name) {
}

Decl::~Decl() {
}

ConstDecl::ConstDecl(const string &name, shared_ptr<Expr> value,
  const location &loc, Indexer&)
  : Decl(name, loc), value(value) {
}

void ConstDecl::validate() const {
    value->validate();
    if (!value->constant())
        throw RumurError("const definition is not a constant", value->loc);
}

void ConstDecl::define(ostream &out) const {
    out << "static int64_t model_" << name << "(const State*s __attribute__((unused))){return ";
    value->rvalue(out);
    out << ";}";
}

TypeDecl::TypeDecl(const string &name, shared_ptr<TypeExpr> value,
  const location &loc, Indexer&)
  : Decl(name, loc), value(value) {
}

void TypeDecl::validate() const {
    value->validate();
}

void TypeDecl::define(ostream &out) const {
    value->define(out);
}

VarDecl::VarDecl(const string &name, shared_ptr<TypeExpr> type,
  const location &loc, Indexer&)
  : Decl(name, loc), type(type) {
}

void VarDecl::define(ostream&) const {
    // TODO
}
