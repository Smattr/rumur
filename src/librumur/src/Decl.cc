#include "location.hh"
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <string>

using namespace rumur;
using namespace std;

Decl::Decl(const string &name, const location &loc)
  : Node(loc), name(name) {
}

ConstDecl::ConstDecl(const string &name, Expr *value, const location &loc)
  : Decl(name, loc), value(value) {
}

ConstDecl::~ConstDecl() {
    delete value;
}

TypeDecl::TypeDecl(const string &name, TypeExpr *value, const location &loc)
  : Decl(name, loc), value(value) {
}

TypeDecl::~TypeDecl() {
    delete value;
}
