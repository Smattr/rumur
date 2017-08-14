#include <algorithm>
#include <cstdint>
#include "location.hh"
#include <memory>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <rumur/Rule.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <unordered_set>
#include <vector>

using namespace rumur;
using namespace std;

Model::Model(vector<shared_ptr<Decl>> &&decls, vector<shared_ptr<Rule>> &&rules,
  const location &loc)
  : Node(loc), decls(decls), rules(rules) {
}

void Model::validate() const {

    // Check all constdecls are actually constant.
    for (const shared_ptr<Decl> d : decls) {
        if (auto c = dynamic_pointer_cast<const ConstDecl>(d)) {
            if (!c->value->constant()) {
                throw RumurError("const definition is not a constant", c->value->loc);
            }
        }
    }

    // Check all range types have constant bounds.
    for (const shared_ptr<Decl> d : decls) {
        if (auto t = dynamic_pointer_cast<const TypeDecl>(d)) {
            if (auto r = dynamic_pointer_cast<const Range>(t->value)) {
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
    auto is_start_state = [](const shared_ptr<Rule> r) {
        return dynamic_pointer_cast<const StartState>(r) != nullptr;
    };
    if (find_if(rules.begin(), rules.end(), is_start_state) == rules.end())
        throw RumurError("model has no start state", location());

    // Check all rule names are distinct.
    unordered_set<string> names;
    for (const shared_ptr<Rule> r : rules) {
        if (r->name != "") {
            if (!names.insert(r->name).second)
                throw RumurError("duplicate rule name " + r->name, r->loc);
        }
    }

}

uint64_t Model::size_bits() const {
    // TODO
    return 1;
}
