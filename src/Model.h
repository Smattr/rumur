#pragma once

#include "Decl.h"
#include <vector>

namespace rumur {

class Model {

  public:

    std::vector<Decl*> decls;

    Model(std::vector<Decl*> &&decls);

    ~Model();

};

}
