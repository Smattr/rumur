#pragma once

#include <cstdint>
#include <string>

namespace rumur {

class Number {

  public:

    int64_t value;

    explicit Number(const std::string &value);
    explicit Number(const Number &other) noexcept;

};

}
