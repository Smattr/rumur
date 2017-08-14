#include "location.hh"
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <string>

using namespace rumur;
using namespace std;

Decl::Decl(const string &name, const location &loc)
  : Node(loc), name(name) {
}

ConstDecl::ConstDecl(const string &name, shared_ptr<Expr> value, const location &loc)
  : Decl(name, loc), value(value) {
}

TypeDecl::TypeDecl(const string &name, shared_ptr<TypeExpr> value, const location &loc)
  : Decl(name, loc), value(value) {
}

VarDecl::VarDecl(const string &name, shared_ptr<TypeExpr> type, const location &loc)
  : Decl(name, loc), type(type) {
}
