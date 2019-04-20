#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include "ValueType.h"

static const std::unordered_map<std::string, ValueType> types = {
  { "int8_t",   { "int8_t",   "INT8_MIN",      "INT8_MAX",   "INT8_C"   } },
  { "uint8_t",  { "uint8_t",  "((uint8_t)0)",  "UINT8_MAX",  "UINT8_C"  } },
  { "int16_t",  { "int16_t",  "INT16_MIN",     "INT16_MAX",  "INT16_C"  } },
  { "uint16_t", { "uint16_t", "((uint16_t)0)", "UINT16_MAX", "UINT16_C" } },
  { "int32_t",  { "int32_t",  "INT32_MIN",     "INT32_MAX",  "INT32_C"  } },
  { "uint32_t", { "uint32_t", "((uint32_t)0)", "UINT32_MAX", "UINT32_C" } },
  { "int64_t",  { "int64_t",  "INT64_MIN",     "INT64_MAX",  "INT64_C"  } },
  { "uint64_t", { "uint64_t", "((uint64_t)0)", "UINT64_MAX", "UINT64_C" } },
};

const ValueType &get_value_type(const std::string &name) {

  auto it = types.find(name);
  if (it != types.end())
    return it->second;

  throw std::runtime_error("unknown type " + name);
}
