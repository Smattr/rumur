#include "location.hh"
#include <rumur/Decl.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <vector>

using namespace rumur;
using namespace std;

Model::Model(vector<Decl*> &&decls, const location &loc)
  : Node(loc), decls(decls) {
}

Model::~Model() {
    for (Decl *decl : decls)
        delete decl;
}
