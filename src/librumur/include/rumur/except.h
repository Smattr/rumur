#pragma once

#include "location.hh"
#include <stdexcept>
#include <string>

namespace rumur {

/* A basic exception to allow us to easily catch only the errors thrown by
 * ourselves.
 */
class RumurError : public std::runtime_error {

 public:
  location loc;

  RumurError(const std::string &message, const location &loc);

};

}
