#pragma once

#include <cstddef>
#include <rumur/Model.h>

#ifndef RUMUR_API
#define RUMUR_API __attribute__((visibility("default")))
#endif

namespace rumur {

/* Resolve symbolic references (rumur::ExprIDs and rumur::TypeExprIDs) within a
 * model. Throws rumur::Error if this process fails.
 */
RUMUR_API void resolve_symbols(Model &m);

} // namespace rumur
