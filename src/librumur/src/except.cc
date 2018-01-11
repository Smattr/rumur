#include "location.hh"
#include <rumur/except.h>
#include <stdexcept>
#include <string>

using namespace std;

namespace rumur {

RumurError::RumurError(const string &message, const location &loc)
  : runtime_error(message), loc(loc) {
}

}
