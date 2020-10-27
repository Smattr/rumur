#include <cstddef>
#include <gmpxx.h>
#include <rumur/Context.h>
#include <rumur/except.h>
#include <rumur/Node.h>
#include <string>
#include <unordered_map>

namespace rumur {

Context::Context() {
  // open a global, top-level scope
  push_scope();
}

void Context::declare(size_t id, const Node &n) {

  // disallow redeclaring the same variable within the same scope
  auto it = scopes.back().find(id);
  if (it != scopes.back().end())
    throw Error("redeclaration of ID " + std::to_string(id), n.loc);

  scopes.back()[id] = Value{false, 0};
}

std::unordered_map<size_t, Context::Value>::const_iterator Context::find(size_t id) const {

  // look through the scopes from inner most to outer most
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    auto i = it->find(id);
    if (i != it->end())
      return i;
  }

  // we did not find it
  return scopes[0].end();
}

void Context::set(size_t id, const mpz_class &value, const Node &n) {

  // repeat find() logic because our 'this' pointer is not const
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    auto i = it->find(id);
    if (i != it->end()) {
      i->second.defined = true;
      i->second.value = value;
      return;
    }
  }

  // we did not find it
  throw Error("setting undeclared ID " + std::to_string(id), n.loc);
}

mpz_class Context::get(size_t id, const Node &n) const {

  auto it = find(id);
  if (it == scopes[0].end())
    throw Error("getting undeclared ID " + std::to_string(id), n.loc);

  if (!it->second.defined)
    throw Error("accessing ID " + std::to_string(id) + " while it is undefined",
      n.loc);

  return it->second.value;
}

bool Context::isundefined(size_t id, const Node &n) const {

  auto it = find(id);
  if (it == scopes[0].end())
    throw Error("getting undeclared ID " + std::to_string(id), n.loc);

  return !it->second.defined;
}

void Context::push_scope() {
  scopes.emplace_back();
}

void Context::pop_scope() {
  if (scopes.size() <= 1)
    throw Error("pop of a context's global scope", location());
  scopes.pop_back();
}

}
