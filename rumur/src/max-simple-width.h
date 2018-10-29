#pragma once

#include <cstddef>
#include <gmpxx.h>
#include <rumur/rumur.h>

/** Get the greatest width of any simple type used in a model.
 *
 * It may not immediately be clear why you would care about such a property.
 * However, note that the only handles that are ever actually *read* or
 * *written* in the generated checker are those for simple typed values. With
 * this in mind, we can conclude that handle_read_raw and handle_write_raw only
 * ever need to deal with handles of at most this width. Knowing this value
 * statically ahead of time enables some interesting optimisations.
 *
 * Returns 0 if there are no simple types in the model. This case should only
 * occur if the state is empty; i.e. if the model has no variables.
 */
mpz_class max_simple_width(const rumur::Model &m);
