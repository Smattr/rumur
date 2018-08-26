#pragma once

#include <rumur/Model.h>

namespace rumur {

/* Validate a model: check it for structural inconsistencies and throw
 * rumur::Error if found.
 */
void validate_model(const Model &m);

}
