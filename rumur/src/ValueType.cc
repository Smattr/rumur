#include <cstddef>
#include <cstdint>
#include <gmpxx.h>
#include <limits.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include "ValueType.h"


static const std::unordered_map<std::string, ValueType> types = {
  { "int8_t",   { "int_fast8_t",   "INT_FAST8_MIN",  "INT_FAST8_MAX",   "INT8_C",   "PRIdFAST8",  mpz_class(std::to_string(INT8_MIN)),  mpz_class(std::to_string(INT8_MAX))   } },
  { "uint8_t",  { "uint_fast8_t",  "((uint8_t)0)",   "UINT_FAST8_MAX",  "UINT8_C",  "PRIuFAST8",  0,                                    mpz_class(std::to_string(UINT8_MAX))  } },
  { "int16_t",  { "int_fast16_t",  "INT_FAST16_MIN", "INT_FAST16_MAX",  "INT16_C",  "PRIdFAST16", mpz_class(std::to_string(INT16_MIN)), mpz_class(std::to_string(INT16_MAX))  } },
  { "uint16_t", { "uint_fast16_t", "((uint16_t)0)",  "UINT_FAST16_MAX", "UINT16_C", "PRIuFAST16", 0,                                    mpz_class(std::to_string(UINT16_MAX)) } },
  { "int32_t",  { "int_fast32_t",  "INT_FAST32_MIN", "INT_FAST32_MAX",  "INT32_C",  "PRIdFAST32", mpz_class(std::to_string(INT32_MIN)), mpz_class(std::to_string(INT32_MAX))  } },
  { "uint32_t", { "uint_fast32_t", "((uint32_t)0)",  "UINT_FAST32_MAX", "UINT32_C", "PRIuFAST32", 0,                                    mpz_class(std::to_string(UINT32_MAX)) } },
  { "int64_t",  { "int_fast64_t",  "INT_FAST64_MIN", "INT_FAST64_MAX",  "INT64_C",  "PRIdFAST64", mpz_class(std::to_string(INT64_MIN)), mpz_class(std::to_string(INT64_MAX))  } },
  { "uint64_t", { "uint_fast64_t", "((uint64_t)0)",  "UINT_FAST64_MAX", "UINT64_C", "PRIuFAST64", 0,                                    mpz_class(std::to_string(UINT64_MAX)) } },
};

ValueType get_value_type(const std::string &name) {

  auto it = types.find(name);
  if (it != types.end())
    return it->second;

  throw std::runtime_error("unknown type " + name);
}
