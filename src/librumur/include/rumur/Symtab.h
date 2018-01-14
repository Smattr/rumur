#pragma once

#include <cassert>
#include "location.hh"
#include <rumur/except.h>
#include <rumur/Node.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace rumur {

class Symtab {

  private:

    std::vector<std::unordered_map<std::string, Node*>> scope;

  public:

    void open_scope() {
        scope.emplace_back();
    }

    void close_scope() {
        assert(!scope.empty());
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
        throw RumurError("unknown symbol: " + name, loc);
    }

};

}
