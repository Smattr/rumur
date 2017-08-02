#pragma once

#include "location.hh"
#include <rumur/Decl.h>
#include <rumur/Node.h>
#include <vector>

namespace rumur {

class Model : public Node {

  public:

    std::vector<Decl*> decls;

    explicit Model(std::vector<Decl*> &&decls, const location &loc);

    ~Model();

};

}
