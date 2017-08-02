#pragma once

#include <cstdint>
#include <rumur/Expr.h>
#include <string>

namespace rumur {

class Number : public Expr {

  public:

    int64_t value;

    explicit Number(const std::string &value);
    explicit Number(const Number &other) noexcept;

};

}
