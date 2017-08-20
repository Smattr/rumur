#pragma once

#include "location.hh"
#include <memory>
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
    std::shared_ptr<Expr> guard;
    std::vector<std::shared_ptr<Decl>> decls;
    std::vector<std::shared_ptr<Stmt>> body;

    explicit Rule(const std::string &name, std::shared_ptr<Expr> guard,
      std::vector<std::shared_ptr<Decl>> &&decls,
      std::vector<std::shared_ptr<Stmt>> &&body, const location &loc);

};

class StartState : public Rule {

  public:

    explicit StartState(const std::string &name,
      std::vector<std::shared_ptr<Decl>> &&decls,
      std::vector<std::shared_ptr<Stmt>> &&body, const location &loc);

};

class Invariant : public Node {

  public:
    std::string name;
    std::shared_ptr<Expr> guard;

    explicit Invariant(const std::string &name, std::shared_ptr<Expr> guard,
      const location &loc);

};

}
