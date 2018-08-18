#pragma once

#include <iostream>
#include <memory>
#include <rumur/Model.h>

namespace rumur {

/* Parse in a model from an input stream. Throws Errors on parsing errors.
 *
 * This is a one-shot parsing API. That is, it fully parses the input model and
 * validates it. If you want to do something more flexible like parse a partial
 * model or tolerate certain types of errors, you may wish to open code your
 * own parsing logic. I recommend you copy the implementation of this function
 * and tweak as desired.
 */
std::shared_ptr<Model> parse(std::istream *input);

}
