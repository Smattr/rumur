#pragma once

#include <cstddef>
#include <iostream>
#include "location.hh"
#include <string>
#include <vector>

namespace rumur {

/// a Murphi source comment
///
/// Note that this is not an AST node (does not inherit from Node) because
/// comments do not fit into a strictly hierarchical AST. They can appear
/// anywhere that is syntactically valid.
struct Comment {

  /// text of the comment
  std::string content;

  /// is this a /* ... */ comment, as opposed to a -- ... comment?
  bool multiline;

  /// position within source file
  location loc;
};

/// parse source code comments from a Murphi file
///
/// \param input Stream to read source from
/// \return List of parsed comments
std::vector<Comment> parse_comments(std::istream &input);

} // namespace rumur
