#pragma once

#include <cstdint>
#include "location.hh"
#include <rumur/Expr.h>
#include <string>

namespace rumur {

class Number : public Expr {

  public:

    int64_t value;

    explicit Number(const std::string &value, const location &loc);
    explicit Number(const Number &other, const location &loc) noexcept;

};

}
