#pragma once

#include <stdexcept>

/* A basic exception to allow us to easily catch only the errors thrown by
 * ourselves.
 */
class RumurError : public std::runtime_error {

  public:

    // Inherit runtime_error's constructor
    using std::runtime_error::runtime_error;

};
