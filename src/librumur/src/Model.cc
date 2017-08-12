#include <algorithm>
#include "location.hh"
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <rumur/Rule.h>
#include <rumur/TypeExpr.h>
#include <vector>

using namespace rumur;
using namespace std;

Model::Model(vector<Decl*> &&decls, vector<Rule*> &&rules, const location &loc)
  : Node(loc), decls(decls), rules(rules) {
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

    // Check all range types have constant bounds.
    for (const Decl *d : decls) {
        if (auto *t = dynamic_cast<const TypeDecl*>(d)) {
            if (auto *r = dynamic_cast<const Range*>(t->value)) {
                if (!r->min->constant()) {
                    throw RumurError("lower bound of range " + t->name +
                        " is not a constant", r->min->loc);
                }
                if (!r->max->constant()) {
                    throw RumurError("upper bound of range " + t->name +
                        " is not a constant", r->max->loc);
                }
            }
        }
    }

    // Check we have at least one start state.
    auto is_start_state = [](const Rule *r) {
        return dynamic_cast<const StartState*>(r) != nullptr;
    };
    if (find_if(rules.begin(), rules.end(), is_start_state) == rules.end())
        throw RumurError("model has no start state", location());

}

Model::~Model() {
    for (Decl *decl : decls)
        delete decl;
    for (Rule *r : rules)
        delete r;
}
