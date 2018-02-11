#pragma once

#include <cassert>
#include <cctype>
#include "location.hh"
#include <rumur/except.h>
#include <rumur/Node.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace rumur {

static inline std::string tolower(const std::string &s) {
  std::string r;
  for (const char &c : s)
    r += ::tolower(c);
  return r;
}

class Symtab {

 private:
  // Case-insensitive string hash
  struct hash {
    std::size_t operator()(const std::string &s) const noexcept {
      return std::hash<std::string>()(tolower(s));
    }
  };

  // Case-insensitive string comparison
  struct equal_to {
    bool operator()(const std::string &a, const std::string &b) const {
      return tolower(a) == tolower(b);
    }
  };

 private:
  std::vector<std::unordered_map<std::string, Node*, hash, equal_to>> scope;

 public:
  void open_scope() {
    scope.emplace_back();
  }

  void close_scope() {
    assert(!scope.empty());
    for (std::pair<std::string, Node*> e : scope[scope.size() - 1])
      delete e.second;
    scope.pop_back();
  }

  void declare(const std::string &name, const Node &value) {
    assert(!scope.empty());
    scope.back()[name] = value.clone();
  }

  template<typename U>
  const U *lookup(const std::string &name, const location &loc) {
    for (auto it = scope.rbegin(); it != scope.rend(); it++) {
      auto it2 = it->find(name);
      if (it2 != it->end()) {
        if (auto ret = dynamic_cast<const U*>(it2->second)) {
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
