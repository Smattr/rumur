#pragma once

#include <cstdint>
#include <iostream>
#include "location.hh"
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
#include <string>

namespace rumur {

class Number : public Expr {

  public:

    int64_t value;

    explicit Number(const std::string &value, const location &loc,
      Indexer &indexer);
    explicit Number(const Number &other, const location &loc,
      Indexer &indexer) noexcept;
    explicit Number(int64_t value, const location &loc, Indexer &indexer)
      noexcept;

    bool constant() const noexcept final;
    const TypeExpr *type() const noexcept final;
    void rvalue(std::ostream &out) const final;

};

}
