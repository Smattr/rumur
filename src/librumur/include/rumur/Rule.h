#pragma once

#include <iostream>
#include "location.hh"
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Indexer.h>
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

    Rule() = delete;
    Rule(const std::string &name, Expr *guard, std::vector<Decl*> &&decls,
      std::vector<Stmt*> &&body, const location &loc,
      Indexer &indexer);
    Rule(const Rule &other);
    Rule &operator=(Rule other);
    friend void swap(Rule &x, Rule &y) noexcept;
    virtual ~Rule();
    Rule *clone() const override;

    void generate_rule(std::ostream &out) const;

};

class StartState : public Rule {

  public:

    StartState() = delete;
    StartState(const std::string &name,
      std::vector<Decl*> &&decls,
      std::vector<Stmt*> &&body, const location &loc,
      Indexer &indexer);
    StartState(const StartState &other) = default;
    StartState(StartState&&) = default;
    StartState &operator=(const StartState&) = default;
    StartState &operator=(StartState&&) = default;
    virtual ~StartState() { }
    StartState *clone() const final;

};

class Invariant : public Node {

  public:
    std::string name;
    Expr *guard;

    Invariant() = delete;
    Invariant(const std::string &name, Expr *guard,
      const location &loc, Indexer &indexer);
    Invariant(const Invariant &other);
    Invariant &operator=(Invariant other);
    friend void swap(Invariant &x, Invariant &y) noexcept;
    virtual ~Invariant();
    Invariant *clone() const final;

};

}
