#include <iostream>
#include "location.hh"
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <string>
#include <vector>

using namespace rumur;
using namespace std;

Rule::Rule(const string &name, shared_ptr<Expr> guard,
  vector<shared_ptr<Decl>> &&decls, vector<shared_ptr<Stmt>> &&body,
  const location &loc, Indexer&)
  : Node(loc), name(name), guard(guard), decls(decls), body(body) {
}

void Rule::generate_rule(ostream &out) const {
    // TODO: decls
    for (const shared_ptr<Stmt> s : body) {
        s->generate_stmt(out);
        out << "\n";
    }
}

/* XXX: Why do we need the explicit moves here? I assumed that we could
 * transparently forward the rvalue references, but it seems they need to be
 * bound to lvalue references which does not happen automatically.
 */
StartState::StartState(const string &name, vector<shared_ptr<Decl>> &&decls,
  vector<shared_ptr<Stmt>> &&body, const location &loc, Indexer& indexer)
  : Rule(name, nullptr, move(decls), move(body), loc, indexer) {
}

Invariant::Invariant(const string &name_, shared_ptr<Expr> guard_,
  const location &loc_, Indexer&)
  : Node(loc_), name(name_), guard(guard_) {
}
