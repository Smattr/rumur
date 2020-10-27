#pragma once

#include <cstddef>
#include <gmpxx.h>
#include <rumur/Node.h>
#include <unordered_map>
#include <vector>

namespace rumur {

/// an environment for compile-time evaluation of Murphi expressions
///
/// Constant folding a simple arithmetic expression involving only constants can
/// be straightforward. However, sometimes it is desirable to fold something
/// more complex. For example, an expression involving a call to a function that
/// can be evaluated at compile-time. To support this, we need a representation
/// of the Murphi execution environment, capturing what symbols are currently in
/// scope and their values. The following class provides this.
class Context {

 private:
  struct Value {
    bool defined;
    mpz_class value;
  };

  /// a stack of symbol scopes in which IDs can be declared
  std::vector<std::unordered_map<size_t, Value>> scopes;

  /// get a handle to a declared symbol
  std::unordered_map<size_t, Value>::const_iterator find(size_t id) const;

 public:
  Context();

  // the following methods take a Node reference simply for attaching source
  // location information to any Errors they throw to help the caller debug

  /// declare the existence of a new symbol in the current scope
  void declare(size_t id, const Node &n);

  /// set the current value of a symbol
  void set(size_t id, const mpz_class &value, const Node &n);

  /// lookup the current value of a symbol
  mpz_class get(size_t id, const Node &n) const;

  /// check if the given declared symbol is currently undefined
  bool isundefined(size_t id, const Node &n) const;

  /// enter or exit a symbol scope
  void push_scope();
  void pop_scope();
};

}
