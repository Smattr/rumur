#include <rumur/Decl.h>
#include <string>

using namespace rumur;
using namespace std;

Decl::Decl(const string &name)
  : name(name) {
}

ConstDecl::ConstDecl(const string &name, Number *value)
  : Decl(name), value(value) {
}

ConstDecl::~ConstDecl() {
    delete value;
}
