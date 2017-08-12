#pragma once

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

    ~Model();

};

}
