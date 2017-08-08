#pragma once

#include <cassert>
#include <rumur/except.h>
#include <stack>
#include <string>
#include <unordered_map>

namespace rumur {

template<typename T>
class Symtab {

  private:

    std::stack<std::unordered_map<std::string, T>> scope;

  public:

    void open_scope() {
        scope.push(std::unordered_map<std::string, T>());
    }

    void close_scope() {
        assert(!scope.empty());
        scope.pop();
    }

    void declare(const std::string &name, T value) {
        assert(!scope.empty());
        scope.top()[name] = value;
    }

    template<typename U>
    U lookup(const std::string &name) {
        for (auto it = scope.rbegin(); it != scope.rend(); it++) {
            auto it2 = it->find(name);
            if (it2 != it->end()) {
                if (auto ret = dynamic_cast<U>(*it2)) {
                    return ret;
                } else {
                    break;
                }
            }
        }
        throw RumurError("unknown symbol: " + name);
    }

};

}
