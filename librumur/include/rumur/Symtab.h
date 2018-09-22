#pragma once

#include <cassert>
#include <cstddef>
#include "location.hh"
#include <memory>
#include <rumur/except.h>
#include <rumur/Node.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace rumur {

class Symtab {

 private:
  std::vector<std::unordered_map<std::string, std::shared_ptr<Node>>> scope;

 public:
  void open_scope() {
    scope.emplace_back();
  }

  void close_scope() {
    assert(!scope.empty());
    scope.pop_back();
  }

  void declare(const std::string &name, std::shared_ptr<Node> value) {
    assert(!scope.empty());
    scope.back()[name] = value;
  }

  template<typename U>
  std::shared_ptr<U> lookup(const std::string &name, const location &loc) {
    for (auto it = scope.rbegin(); it != scope.rend(); it++) {
      auto it2 = it->find(name);
      if (it2 != it->end()) {
        if (auto ret = std::dynamic_pointer_cast<U>(it2->second)) {
          return ret;
        } else {
          break;
        }
      }
    }
    throw Error("unknown symbol: " + name, loc);
  }

  ~Symtab() {
    while (scope.size() > 0)
      close_scope();
  }

  // Whether we are in the top-level scope.
  bool is_global_scope() const {
    return scope.size() == 1;
  }

};

}
