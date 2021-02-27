#pragma once

#include <cstddef>
#include <rumur/Model.h>

namespace rumur {

/* Resolve symbolic references (rumur::ExprIDs and rumur::TypeExprIDs) within a
 * model. Throws rumur::Error if this process fails.
 */
void resolve_symbols(Model &m);

} // namespace rumur
