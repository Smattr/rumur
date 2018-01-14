#include "location.hh"
#include <rumur/except.h>
#include <stdexcept>
#include <string>

namespace rumur {

RumurError::RumurError(const std::string &message, const location &loc)
  : runtime_error(message), loc(loc) {
}

}
