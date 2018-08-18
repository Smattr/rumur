#pragma once

#include <iostream>
#include "location.hh"
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/Property.h>
#include <rumur/Stmt.h>
#include <string>
#include <vector>

namespace rumur {

struct Rule : public Node {

  std::string name;
  std::vector<Quantifier*> quantifiers;

  Rule() = delete;
  Rule(const std::string &name_, const location &loc_);
  Rule(const Rule &other);

  Rule *clone() const override = 0;

  virtual std::vector<std::shared_ptr<Rule>> flatten() const;

  virtual ~Rule();
};

struct SimpleRule : public Rule {

  Expr *guard;
  std::vector<Decl*> decls;
  std::vector<std::shared_ptr<Stmt>> body;

  SimpleRule() = delete;
  SimpleRule(const std::string &name_, Expr *guard_, std::vector<Decl*> &&decls_,
    std::vector<std::shared_ptr<Stmt>> &&body_, const location &loc_);
  SimpleRule(const SimpleRule &other);
  SimpleRule &operator=(SimpleRule other);
  friend void swap(SimpleRule &x, SimpleRule &y) noexcept;
  virtual ~SimpleRule();
  SimpleRule *clone() const override;
  bool operator==(const Node &other) const override;
  void validate() const final;
};

struct StartState : public Rule {

  std::vector<Decl*> decls;
  std::vector<std::shared_ptr<Stmt>> body;

  StartState() = delete;
  StartState(const std::string &name_, std::vector<Decl*> &&decls_,
    std::vector<std::shared_ptr<Stmt>> &&body_, const location &loc_);
  StartState(const StartState &other);
  StartState &operator=(StartState other);
  friend void swap(StartState &x, StartState &y) noexcept;
  virtual ~StartState();
  StartState *clone() const final;
  bool operator==(const Node &other) const final;
  void validate() const final;
};

struct PropertyRule : public Rule {

  Property property;

  PropertyRule() = delete;
  PropertyRule(const std::string &name_, const Property &property_,
    const location &loc_);
  PropertyRule(const PropertyRule &other);
  PropertyRule &operator=(PropertyRule other);
  friend void swap(PropertyRule &x, PropertyRule &y) noexcept;
  virtual ~PropertyRule() { }
  PropertyRule *clone() const final;
  bool operator==(const Node &other) const final;
};

struct Ruleset : public Rule {

  std::vector<std::shared_ptr<Rule>> rules;

  Ruleset() = delete;
  Ruleset(std::vector<Quantifier*> &&quantifiers_,
    std::vector<std::shared_ptr<Rule>> &&rules_, const location &loc_);
  Ruleset(const Ruleset &other);
  Ruleset &operator=(Ruleset other);
  friend void swap(Ruleset &x, Ruleset &y) noexcept;
  virtual ~Ruleset() { }
  Ruleset *clone() const final;
  bool operator==(const Node &other) const final;

  std::vector<std::shared_ptr<Rule>> flatten() const final;
};

}
