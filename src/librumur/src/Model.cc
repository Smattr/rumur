#include "location.hh"
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <vector>

using namespace rumur;
using namespace std;

Model::Model(vector<Decl*> &&decls, const location &loc)
  : Node(loc), decls(decls) {
}

void Model::validate() const {

    // Check all constdecls are actually constant.
    for (const Decl *d : decls) {
        if (auto *c = dynamic_cast<const ConstDecl*>(d)) {
            if (!c->value->constant()) {
                throw RumurError("const definition is not a constant", c->value->loc);
            }
        }
    }

}

Model::~Model() {
    for (Decl *decl : decls)
        delete decl;
}
