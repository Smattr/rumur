// functionality related to interacting with a Satisfiability Modulo Theories
// solver

#pragma once

#include <cstddef>
#include <gmpxx.h>
#include <rumur/Node.h>
#include <rumur/Number.h>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace rumur {

// Symbolic context, for maintaining a mapping between Murphi variables and
// external (generated) symbols. This has extremely limited functionality, only
// enough to support the translation to SMT.
class SMTContext {

 private:
  // use bitvectors instead of integers for numeric values?
  bool prefer_bitvectors = false;

  // bit width to use to represent numerical values if using bitvectors
  size_t bitvector_width = 64;

  // stack of symbol table entries, mapping AST unique IDs to external names
  std::vector<std::pair<size_t, std::string>> scope;

  // monotonic counter used for generating unique symbols
  size_t counter = 0;

  // accrued SMTLIB problem
  std::ostringstream content;

 public:
  SMTContext();
  SMTContext(bool prefer_bitvectors_, size_t bitvector_width_);

  /// add a new known symbol
  ///
  /// This registers the symbol in the current innermost scope.
  ///
  /// \param id Unique identifier of the source AST node
  /// \return A unique name created for this symbol
  std::string register_symbol(size_t id);

  /// lookup a previously registered symbol
  ///
  /// This lookup is performed in all known variable scopes, going from
  /// innermost to outermost in preference order
  ///
  /// \param id Unique identifier of the AST node being looked up
  /// \param origin The node that caused this lookup (used for error
  ///   diagnostics)
  /// \return The unique name this symbol maps to
  std::string lookup_symbol(size_t id, const Node &origin) const;

  /// invent a new SMT type
  ///
  /// This alternative is for defining an anonymous (unreferencable in Rumur)
  /// type.
  ///
  /// \param type The native (or previously defined) SMT type that this should
  ///   alias
  /// \return A unique name for this type
  std::string make_type(const std::string &type);

  /// invent a new SMT type
  ///
  /// This alternative is for when the type being defined corresponds to a
  /// referencable (named) Rumur AST node.
  ///
  /// \param id Unique identifier of the TypeDecl motivating this definition
  /// \param type The native (or previously defined) SMT type that this should
  ///   alias
  /// \return A unique name for this type
  std::string make_type(size_t id, const std::string &type);

  // various SMT operators whose selection is dependent on context
  std::string add (const Node &origin) const;
  std::string band(const Node &origin) const;
  std::string bnot(const Node &origin) const;
  std::string bor (const Node &origin) const;
  std::string bxor(const Node &origin) const;
  std::string div (const Node &origin) const;
  std::string geq (const Node &origin) const;
  std::string gt  (const Node &origin) const;
  std::string leq (const Node &origin) const;
  std::string lsh (const Node &origin) const;
  std::string lt  (const Node &origin) const;
  std::string mod (const Node &origin) const;
  std::string mul (const Node &origin) const;
  std::string neg (const Node &origin) const;
  std::string rsh (const Node &origin) const;
  std::string sub (const Node &origin) const;
  std::string numeric_literal(const mpz_class &value, const Number &origin) const;

  SMTContext &operator<<(const std::string &s);

 private:
  std::string make_symbol();

};

// translate a given expression to SMTLIBv2
void to_smt(std::ostream &out, const Node &n, SMTContext &ctxt);

// wrapper around the above for when you need a string
std::string to_smt(const Node &n, SMTContext &ctxt);

}
