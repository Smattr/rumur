#pragma once

#include <cstddef>
#include <iostream>
#include <rumur/rumur.h>

/** translate a Murphi AST node to Uclid5 code
 *
 * This function may throw rumur::Errors when encountering unsupported nodes.
 * Desired indentation is assumed to be two spaces with a starting indentation
 * level 1. I.e. it is assumed that the caller is surrounding this with a
 * `module { ... }` block.
 *
 * \param n The node to translate
 * \param out Stream to write the translation to
 */
void codegen(const rumur::Node &n, std::ostream &out);
