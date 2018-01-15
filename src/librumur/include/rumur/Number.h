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

    Number() = delete;
    Number(const std::string &value, const location &loc, Indexer &indexer);
    Number(int64_t value, const location &loc, Indexer &indexer);
    Number(const Number&) = default;
    Number(Number&&) = default;
    Number &operator=(const Number&) = default;
    Number &operator=(Number&&) = default;
    virtual ~Number() { }
    Number *clone() const final;

    bool constant() const final;
    const TypeExpr *type() const final;
    void rvalue(std::ostream &out) const final;
    void generate(std::ostream &out) const final;

};

}
