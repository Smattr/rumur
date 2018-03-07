#pragma once

#include <rumur/Model.h>
#include <string>

namespace rumur {

struct OutputOptions {
  bool overflow_checks;
  unsigned long threads;
  bool debug;
  size_t set_capacity;

  // Use Intel Thread Building Blocks
  bool tbb;
};

int output_checker(const std::string &path, const Model &model,
  const OutputOptions &options);

}
