#pragma once

#include <iostream>
#include "location.hh"

namespace rumur {

class Node {

 public:
  location loc;

  Node() = delete;
  Node(const location &loc_);
  Node(const Node&) = default;
  Node(Node&&) = default;
  Node &operator=(const Node&) = default;
  Node &operator=(Node&&) = default;
  virtual ~Node() { }

  virtual Node *clone() const = 0;

  virtual bool operator==(const Node &other) const = 0;
  bool operator!=(const Node &other) const {
    return !(*this == other);
  }

};

}
