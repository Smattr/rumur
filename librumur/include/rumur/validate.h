#pragma once

#include <cstddef>
#include <rumur/Model.h>
#include <rumur/Node.h>

namespace rumur {

/* Check a node in the AST and all its children for inconsistencies and throw
 * rumur::Error if found.
 */
void validate(const Node &n);

} // namespace rumur
