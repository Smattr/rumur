#include <cstddef>
#include "../../common/escape.h"
#include "options.h"
#include <rumur/rumur.h>
#include <sstream>
#include <string>
#include "utils.h"

using namespace rumur;

std::string to_C_string(const Expr &expr) {
  return "\"" + escape(expr.to_string()) + "\"";
}

static std::string to_string(const location &location) {
  std::stringstream ss;
  ss << location;
  return ss.str();
}

std::string to_C_string(const location &location) {
  return "\"" + escape(input_filename) + ":" + to_string(location) + ": \"";
}
