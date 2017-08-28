#include <algorithm>
#include <cstdint>
#include "location.hh"
#include <memory>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Indexer.h>
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
  const location &loc, Indexer&)
  : Node(loc), decls(decls), rules(rules) {
}

void Model::validate() const {

    for (const shared_ptr<Decl> d : decls)
        d->validate();

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
