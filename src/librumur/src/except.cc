#include "location.hh"
#include <rumur/except.h>
#include <stdexcept>
#include <string>

using namespace rumur;
using namespace std;

RumurError::RumurError(const string &message, const location &loc)
  : runtime_error(message), loc(loc) {
}
