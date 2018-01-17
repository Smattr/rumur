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

  virtual void validate() const {  }

  virtual Node *clone() const = 0;

  virtual void generate(std::ostream &out) const = 0;

};

static inline std::ostream &operator<<(std::ostream &out, const Node &n) {
  n.generate(out);
  return out;
}

}
