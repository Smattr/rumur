#pragma once

#include <iostream>
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

  Rule() = delete;
  Rule(const std::string &name_, const location &loc_);
  Rule(const Rule &other);

  Rule *clone() const override = 0;

  virtual ~Rule() { }
};

class SimpleRule : public Rule {

 public:
  Expr *guard;
  std::vector<Decl*> decls;
  std::vector<Stmt*> body;
  std::vector<Quantifier*> quantifiers;

  SimpleRule() = delete;
  SimpleRule(const std::string &name_, Expr *guard_, std::vector<Decl*> &&decls_,
    std::vector<Stmt*> &&body_, const location &loc_);
  SimpleRule(const SimpleRule &other);
  SimpleRule &operator=(SimpleRule other);
  friend void swap(SimpleRule &x, SimpleRule &y) noexcept;
  virtual ~SimpleRule();
  SimpleRule *clone() const override;
  bool operator==(const Node &other) const override;
};

class StartState : public Rule {

 public:
  std::vector<Decl*> decls;
  std::vector<Stmt*> body;

  StartState() = delete;
  StartState(const std::string &name_, std::vector<Decl*> &&decls_,
    std::vector<Stmt*> &&body_, const location &loc_);
  StartState(const StartState &other);
  StartState &operator=(StartState other);
  friend void swap(StartState &x, StartState &y) noexcept;
  virtual ~StartState() { }
  StartState *clone() const final;
  bool operator==(const Node &other) const final;
};

class Invariant : public Rule {

 public:
  Expr *guard;

  Invariant() = delete;
  Invariant(const std::string &name_, Expr *guard_,
    const location &loc_);
  Invariant(const Invariant &other);
  Invariant &operator=(Invariant other);
  friend void swap(Invariant &x, Invariant &y) noexcept;
  virtual ~Invariant();
  Invariant *clone() const final;
  bool operator==(const Node &other) const final;
};

class Ruleset : public Rule {

 public:
  std::vector<Quantifier*> quantifiers;
  std::vector<Rule*> rules;

  Ruleset() = delete;
  Ruleset(std::vector<Quantifier*> &&quantifiers_, std::vector<Rule*> &&rules_,
    const location &loc_);
  Ruleset(const Ruleset &other);
  Ruleset &operator=(Ruleset other);
  friend void swap(Ruleset &x, Ruleset &y) noexcept;
  virtual ~Ruleset();
  Ruleset *clone() const final;
  bool operator==(const Node &other) const final;
};

}
