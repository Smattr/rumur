#pragma once

#include <cstddef>
#include <iostream>
#include <rumur/rumur.h>
#include <vector>

/** translate a Murphi AST node to Uclid5 code
 *
 * This function may throw rumur::Errors when encountering unsupported nodes.
 * Desired indentation is assumed to be two spaces with a starting indentation
 * level 0.
 *
 * \param n The node to translate
 * \param comments Comments parsed from the source code
 * \param out Stream to write the translation to
 */
void codegen(const rumur::Node &n, const std::vector<rumur::Comment> &comments,
             std::ostream &out);
