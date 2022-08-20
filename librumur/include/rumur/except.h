#pragma once

#include "location.hh"
#include <cstddef>
#include <stdexcept>
#include <string>

#ifndef RUMUR_API_WITH_RTTI
#define RUMUR_API_WITH_RTTI __attribute__((visibility("default")))
#endif

namespace rumur {

/* A basic exception to allow us to easily catch only the errors thrown by
 * ourselves.
 */
class RUMUR_API_WITH_RTTI Error : public std::runtime_error {

public:
  location loc;

  Error(const std::string &message, const location &loc_);
  Error(const std::string &prefix, const Error &sub);
};

} // namespace rumur
