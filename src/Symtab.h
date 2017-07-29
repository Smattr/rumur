#include <cassert>
#include "except.h"
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
        scope.push();
    }

    void close_scope() {
        assert(!scope.empty());
        scope.pop();
    }

    void declare(const std::string &name, T value) {
        assert(!scope.empty());
        scope.top()[name] = value;
    }

    T lookup(const std::string &name) {
        for (auto it = scope.rbegin(); it != scope.rend(); it++) {
            auto it2 = it->find(name);
            if (it2 != it->end())
                return *it2;
        }
        throw RumurError("unknown symbol: " + name);
    }

};

}
