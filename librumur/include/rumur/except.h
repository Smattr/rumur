#pragma once

#include <cstddef>
#include "location.hh"
#include <stdexcept>
#include <string>

namespace rumur {

/* A basic exception to allow us to easily catch only the errors thrown by
 * ourselves.
 */
class Error : public std::runtime_error {

 public:
  location loc;

  Error(const std::string &message, const location &loc_);
  Error(const std::string &prefix, const Error &sub);

};

}
