#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include "ValueType.h"

static const std::unordered_map<std::string, ValueType> types = {
  { "int8_t",   { "int_fast8_t",   "INT_FAST8_MIN",  "INT_FAST8_MAX",   "INT8_C"   } },
  { "uint8_t",  { "uint_fast8_t",  "((uint8_t)0)",   "UINT_FAST8_MAX",  "UINT8_C"  } },
  { "int16_t",  { "int_fast16_t",  "INT_FAST16_MIN", "INT_FAST16_MAX",  "INT16_C"  } },
  { "uint16_t", { "uint_fast16_t", "((uint16_t)0)",  "UINT_FAST16_MAX", "UINT16_C" } },
  { "int32_t",  { "int_fast32_t",  "INT_FAST32_MIN", "INT_FAST32_MAX",  "INT32_C"  } },
  { "uint32_t", { "uint_fast32_t", "((uint32_t)0)",  "UINT_FAST32_MAX", "UINT32_C" } },
  { "int64_t",  { "int_fast64_t",  "INT_FAST64_MIN", "INT_FAST64_MAX",  "INT64_C"  } },
  { "uint64_t", { "uint_fast64_t", "((uint64_t)0)",  "UINT_FAST64_MAX", "UINT64_C" } },
};

const ValueType &get_value_type(const std::string &name) {

  auto it = types.find(name);
  if (it != types.end())
    return it->second;

  throw std::runtime_error("unknown type " + name);
}
