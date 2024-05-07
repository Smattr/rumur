#pragma once

#include "location.hh"
#include <climits>
#include <cstddef>
#include <cstdint>
#include <iostream>

#ifndef RUMUR_API_WITH_RTTI
#define RUMUR_API_WITH_RTTI __attribute__((visibility("default")))
#endif

namespace rumur {

class BaseTraversal;
class ConstBaseTraversal;

struct RUMUR_API_WITH_RTTI Node {

  /// originating line and column source position
  location loc;

  /// A numeric identifier of this node, unique within the current AST. A value
  /// of `SIZE_MAX` means this node (and its children) have not yet been
  /// indexed (see `Model::reindex`). This value can be useful when doing code
  /// generation and needing to invent unique symbols.
  size_t unique_id = SIZE_MAX;

  Node() = delete;
  Node(const location &loc_);
  virtual ~Node() = default;

  /// Create a copy of this node. This is necessary rather than simply relying
  /// on the copy constructor because of the inheritance hierarchy. That is, an
  /// agnostic class like `Ptr` can call this to copy a node without knowing
  /// its precise derived type and without object slicing.
  virtual Node *clone() const = 0;

  /// Confirm that data structure invariants hold. This function throws
  /// `rumur::Error` if invariants are violated.
  virtual void validate() const {}

  /// traverse this node and its children using the given action
  virtual void visit(BaseTraversal &visitor) = 0;
  virtual void visit(ConstBaseTraversal &visitor) const = 0;

protected:
  Node(const Node &) = default;
  Node &operator=(const Node &) = default;
};

} // namespace rumur
