#pragma once

#include <cstddef>
#include <rumur/Model.h>
#include <rumur/Node.h>

#ifndef RUMUR_API
#define RUMUR_API __attribute__((visibility("default")))
#endif

namespace rumur {

/* Check a node in the AST and all its children for inconsistencies and throw
 * `rumur::Error` if found.
 */
RUMUR_API void validate(const Node &n);

} // namespace rumur
