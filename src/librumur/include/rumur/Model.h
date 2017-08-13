#pragma once

#include <cstdint>
#include "location.hh"
#include <rumur/Decl.h>
#include <rumur/Node.h>
#include <rumur/Rule.h>
#include <vector>

namespace rumur {

class Model : public Node {

  public:
    std::vector<Decl*> decls;
    std::vector<Rule*> rules;

    explicit Model(std::vector<Decl*> &&decls, std::vector<Rule*> &&rules,
      const location &loc);

    void validate() const;

    // Get the size of the state data in bits.
    uint64_t size_bits() const;

    ~Model();

};

}
