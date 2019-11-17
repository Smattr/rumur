#pragma once

#include <climits>
#include <cstddef>
#include <iostream>
#include "location.hh"
#include <vector>

namespace rumur {

struct Node {

  location loc;
  size_t unique_id = SIZE_MAX;

  Node() = delete;
  Node(const location &loc_);
  virtual ~Node() = default;

  virtual Node *clone() const = 0;

  virtual bool operator==(const Node &other) const = 0;
  bool operator!=(const Node &other) const {
    return !(*this == other);
  }

  /* Confirm that data structure invariants hold. This function throws
   * rumur::Errors if invariants are violated.
   */
  virtual void validate() const { }

  // Iteration-supporting infrastructure. You do not need to pay much attention
  // to the following details, but their purpose is to allow you to use C++11
  // range-based for loops on AST nodes:
  //
  //   for (Node *n : myAST) {
  //     ...
  //   }
  //
  // The traversal order is not guaranteed, so do not use this API if you
  // specifically require pre-order or post-order.

  class Iterator {

   private:
    std::vector<Node*> remaining;

   public:
    explicit Iterator();
    explicit Iterator(Node &base);
    Iterator &operator++();
    Iterator operator++(int);
    Node *operator*();
    bool operator==(const Iterator &other) const;
    bool operator!=(const Iterator &other) const;
  };

  Iterator begin();
  Iterator end();

  class ConstIterator {

   private:
    std::vector<const Node*> remaining;

   public:
    explicit ConstIterator();
    explicit ConstIterator(const Node &base);
    ConstIterator &operator++();
    ConstIterator operator++(int);
    const Node *operator*();
    bool operator==(const ConstIterator &other) const;
    bool operator!=(const ConstIterator &other) const;
  };

  ConstIterator begin() const;
  ConstIterator end() const;

};

}
