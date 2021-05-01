#pragma once

#include <cstddef>
#include <rumur/Node.h>

#ifndef RUMUR_API
#define RUMUR_API __attribute__((visibility("default")))
#endif

namespace rumur {

/** ensure all rules have [a-zA-Z_][a-zA-Z_0-9]* names
 *
 * This will rename any rules that are not safe to use as, e.g. a C symbol. This
 * can be useful for code generators that want to derive symbols to emit from
 * the names of the rules.
 */
RUMUR_API void sanitise_rule_names(Node &n);

} // namespace rumur
