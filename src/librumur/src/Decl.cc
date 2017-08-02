#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <string>

using namespace rumur;
using namespace std;

Decl::Decl(const string &name)
  : name(name) {
}

ConstDecl::ConstDecl(const string &name, Expr *value)
  : Decl(name), value(value) {
}

ConstDecl::~ConstDecl() {
    delete value;
}
