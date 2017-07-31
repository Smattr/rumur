#include <rumur/Decl.h>
#include <rumur/Model.h>
#include <vector>

using namespace rumur;
using namespace std;

Model::Model(vector<Decl*> &&decls)
  : decls(decls) {
}

Model::~Model() {
    for (Decl *decl : decls)
        delete decl;
}
