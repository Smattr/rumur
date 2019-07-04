#pragma once

#include <cstddef>
#include <rumur/Model.h>
#include <rumur/Node.h>

namespace rumur {

/* Validate a model: check it for structural inconsistencies and throw
 * rumur::Error if found.
 */
void validate_model(const Model &m);

/* Check a node in the AST and all its children for inconsistencies and throw
 * rumur::Error if found.
 */
void validate(const Node &n);

}
