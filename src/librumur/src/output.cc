#include <iostream>
#include <fstream>
#include <rumur/Model.h>
#include <rumur/output.h>
#include "resources.h"
#include <string>
#include <utility>
#include <vector>

namespace rumur {

// Whether a rule is a standard state transition rule.
int output_checker(const std::string &path, const Model &model,
  const OutputOptions &options) {

  std::ofstream out(path);
  if (!out)
    return -1;

  out

    // Settings that are used in header.cc
    << "static constexpr bool OVERFLOW_CHECKS_ENABLED = " <<
    (options.overflow_checks ? "true" : "false") << ";\n"

    // Static boiler plate code
    << std::string((const char*)resources_header_cc, (size_t)resources_header_cc_len)
    << "\n"

    // the model itself
    << model

    // Final boiler plate
    << std::string((const char*)resources_footer_cc, (size_t)resources_footer_cc_len);

  return 0;

}

}
