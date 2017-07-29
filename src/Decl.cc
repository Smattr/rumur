#include "Decl.h"
#include <string>

using namespace rumur;
using namespace std;

Decl::Decl(const string &name)
  : name(name) {
}

ConstDecl::ConstDecl(const string &name, const Number &value)
  : Decl(name), value(value) {
}
