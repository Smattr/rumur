#pragma once

#include <cstddef>
#include <rumur/Model.h>

#ifndef RUMUR_API
#define RUMUR_API __attribute__((visibility("default")))
#endif

namespace rumur {

/** resolve symbolic references within a model
 *
 * Resolve symbolic references (\p rumur::ExprIDs and \p rumur::TypeExprIDs)
 * within a model. Throws \p rumur::Error if this process fails.
 */
RUMUR_API void resolve_symbols(Model &m);

} // namespace rumur
