#pragma once

#include <cstdint>
#include "location.hh"
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Node.h>
#include <rumur/Rule.h>
#include <vector>

namespace rumur {

class Model : public Node {

  public:
    std::vector<std::shared_ptr<Decl>> decls;
    std::vector<std::shared_ptr<Rule>> rules;

    explicit Model(std::vector<std::shared_ptr<Decl>> &&decls,
      std::vector<std::shared_ptr<Rule>> &&rules, const location &loc);

    void validate() const;

    // Get the size of the state data in bits.
    uint64_t size_bits() const;

};

}
