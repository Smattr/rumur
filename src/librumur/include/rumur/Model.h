#pragma once

#include <rumur/Decl.h>
#include <vector>

namespace rumur {

class Model {

  public:

    std::vector<Decl*> decls;

    explicit Model(std::vector<Decl*> &&decls);

    ~Model();

};

}
