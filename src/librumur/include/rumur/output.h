#pragma once

#include <rumur/Model.h>
#include <string>

namespace rumur {

struct OutputOptions {
    bool overflow_checks;
};

int output_checker(const std::string &path, const Model &model,
  const OutputOptions &options);

}
