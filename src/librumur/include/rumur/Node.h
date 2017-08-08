#pragma once

#include "location.hh"

namespace rumur {

class Node {

  public:
    location loc;

    explicit Node(const location &loc) noexcept;

    virtual ~Node() {
    }

};

}
