#pragma once

#include <cstddef>
#include <iostream>
#include "location.hh"
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/Property.h>
#include <rumur/Ptr.h>
#include <rumur/Stmt.h>
#include <string>
#include <vector>

namespace rumur {

struct Rule : public Node {

  std::string name;
  std::vector<Quantifier> quantifiers;
  std::vector<std::shared_ptr<AliasDecl>> aliases;

  Rule() = delete;
  Rule(const std::string &name_, const location &loc_);
  Rule(const Rule &other);

  Rule *clone() const override = 0;

  virtual std::vector<Ptr<Rule>> flatten() const;

  virtual ~Rule() = default;
};

struct AliasRule : public Rule {
  std::vector<Ptr<Rule>> rules;

  AliasRule() = delete;
  AliasRule(std::vector<std::shared_ptr<AliasDecl>> &&aliases_,
    const std::vector<Ptr<Rule>> &rules_, const location &loc_);
  AliasRule(const AliasRule &other);
  AliasRule &operator=(AliasRule other);
  friend void swap(AliasRule &x, AliasRule &y) noexcept;
  virtual ~AliasRule() = default;
  AliasRule *clone() const final;
  bool operator==(const Node &other) const final;

  std::vector<Ptr<Rule>> flatten() const final;
};

struct SimpleRule : public Rule {

  std::shared_ptr<Expr> guard;
  std::vector<std::shared_ptr<Decl>> decls;
  std::vector<Ptr<Stmt>> body;

  SimpleRule() = delete;
  SimpleRule(const std::string &name_, std::shared_ptr<Expr> guard_,
    std::vector<std::shared_ptr<Decl>> &&decls_,
    const std::vector<Ptr<Stmt>> &body_, const location &loc_);
  SimpleRule(const SimpleRule &other);
  SimpleRule &operator=(SimpleRule other);
  friend void swap(SimpleRule &x, SimpleRule &y) noexcept;
  virtual ~SimpleRule() = default;
  SimpleRule *clone() const override;
  bool operator==(const Node &other) const override;
  void validate() const final;
};

struct StartState : public Rule {

  std::vector<std::shared_ptr<Decl>> decls;
  std::vector<Ptr<Stmt>> body;

  StartState() = delete;
  StartState(const std::string &name_,
    std::vector<std::shared_ptr<Decl>> &&decls_,
    const std::vector<Ptr<Stmt>> &body_, const location &loc_);
  StartState(const StartState &other);
  StartState &operator=(StartState other);
  friend void swap(StartState &x, StartState &y) noexcept;
  virtual ~StartState() = default;
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
  virtual ~PropertyRule() = default;
  PropertyRule *clone() const final;
  bool operator==(const Node &other) const final;
};

struct Ruleset : public Rule {

  std::vector<Ptr<Rule>> rules;

  Ruleset() = delete;
  Ruleset(const std::vector<Quantifier> &quantifiers_,
    const std::vector<Ptr<Rule>> &rules_, const location &loc_);
  Ruleset(const Ruleset &other);
  Ruleset &operator=(Ruleset other);
  friend void swap(Ruleset &x, Ruleset &y) noexcept;
  virtual ~Ruleset() = default;
  Ruleset *clone() const final;
  bool operator==(const Node &other) const final;

  std::vector<Ptr<Rule>> flatten() const final;
};

}
