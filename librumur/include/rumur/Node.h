#pragma once

#include <climits>
#include <iostream>
#include "location.hh"

namespace rumur {

struct Node {

  location loc;
  size_t unique_id = SIZE_MAX;

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

  /* Confirm that data structure invariants hold. This function throws
   * rumur::Errors if invariants are violated.
   */
  virtual void validate() const { }

};

}
