#pragma once

#include "location.hh"

namespace rumur {

class Node {

  public:
    location loc;

    Node() = delete;
    Node(const location &loc);
    Node(const Node&) = default;
    Node(Node&&) = default;
    Node &operator=(const Node&) = default;
    Node &operator=(Node&&) = default;
    virtual ~Node() { }

    virtual void validate() const {  }

    virtual Node *clone() const = 0;

};

}
