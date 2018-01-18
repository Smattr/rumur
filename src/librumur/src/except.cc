#include "location.hh"
#include <rumur/except.h>
#include <stdexcept>
#include <string>

namespace rumur {

RumurError::RumurError(const std::string &message, const location &loc_):
  std::runtime_error(message), loc(loc_) {
}

RumurError::RumurError(const std::string &prefix, const RumurError &sub):
  std::runtime_error(prefix + sub.what()), loc(sub.loc) { }

}
