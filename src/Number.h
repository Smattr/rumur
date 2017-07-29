#pragma once

#include <cstdint>
#include <string>

namespace rumur {

class Number {

  public:

    int64_t value;

    Number(const std::string &value);
    Number(const Number &other) noexcept;

};

}
