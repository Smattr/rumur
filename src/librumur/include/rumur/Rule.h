#pragma once

#include "location.hh"
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/Stmt.h>
#include <string>
#include <vector>

namespace rumur {

class Rule : public Node {

  public:
    std::string name;
    Expr *guard;
    std::vector<Decl*> decls;
    std::vector<Stmt*> body;

    explicit Rule(const std::string &name, Expr *guard,
      std::vector<Decl*> &&decls, std::vector<Stmt*> &&body,
      const location &loc);

    virtual ~Rule();

};

class StartState : public Rule {

  public:

    explicit StartState(const std::string &name, std::vector<Decl*> &&decls,
      std::vector<Stmt*> &&body, const location &loc);

};

}
