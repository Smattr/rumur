#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <string>

namespace rumur {

// A possibly unresolved pointer to a Node in the AST.
template<typename T>
struct Reference {

  // The name that was seen during parsing.
  std::string identifier;

  /* The entity this reference refers to. Will be NULL if this reference is
   * unresolved.
   */
  std::shared_ptr<T> referent;

  Reference() = delete;
  Reference(const std::string &identifier_): identifier(identifier_) { }
  Reference(const Reference&) = default;
  Reference(Reference&&) = default;

  void resolve(std::shared_ptr<T> ref) {
    assert(referent == nullptr &&
      "resolving a reference that was already resolved");
    referent = ref;
  }

  bool resolved() const {
    return referent != nullptr;
  }

};

}
