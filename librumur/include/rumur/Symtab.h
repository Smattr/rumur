#pragma once

#include <cassert>
#include <cstddef>
#include "location.hh"
#include <memory>
#include <rumur/except.h>
#include <rumur/Node.h>
#include <rumur/Ptr.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace rumur {

class Symtab {

 private:
  std::vector<std::unordered_map<std::string, Ptr<Node>>> scope;

 public:
  void open_scope() {
    scope.emplace_back();
  }

  void close_scope() {
    assert(!scope.empty());
    scope.pop_back();
  }

  void declare(const std::string &name, const Ptr<Node> &value) {
    assert(!scope.empty());
    if (scope.back().count(name) > 0)
      throw Error("symbol \"" + name + "\" was previously declared",
        value->loc);
    scope.back()[name] = value;
  }

  template<typename U>
  Ptr<U> lookup(const std::string &name, const location &loc) const {
    for (auto it = scope.rbegin(); it != scope.rend(); it++) {
      auto it2 = it->find(name);
      if (it2 != it->end()) {
        if (auto ret = dynamic_cast<const U*>(it2->second.get())) {
          return Ptr<U>(ret->clone());
        } else {
          break;
        }
      }
    }
    throw Error("unknown symbol: " + name, loc);
  }

  // Whether we are in the top-level scope.
  bool is_global_scope() const {
    return scope.size() == 1;
  }

};

}
