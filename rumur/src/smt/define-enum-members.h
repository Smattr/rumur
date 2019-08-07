#pragma once

#include <cstddef>
#include <iostream>
#include <rumur/rumur.h>

namespace smt {

/* declare any enum members that are syntactically contained under the given
 * type
 */
void define_enum_members(std::ostream &out, const rumur::TypeExpr &type);

}
