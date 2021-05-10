#pragma once

#include <rumur/rumur.h>

/** validate that the given node is translatable to Uclid5
 *
 * This function recursively scans a Murphi AST node and throws a rumur::Error
 * if it encounters anything for which we know we have no Uclid5 equivalent.
 *
 * \param n The node to scan
 */
void check(const rumur::Node &n);
