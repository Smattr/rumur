#pragma once

#include <cstddef>
#include "location.hh"
#include <iostream>
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/Property.h>
#include <rumur/Ptr.h>
#include <rumur/Stmt.h>
#include <string>
#include <vector>

#ifndef RUMUR_API_WITH_RTTI
#define RUMUR_API_WITH_RTTI __attribute__((visibility("default")))
#endif

namespace rumur {

struct RUMUR_API_WITH_RTTI Rule : public Node {

  std::string name;
  std::vector<Quantifier> quantifiers;
  std::vector<Ptr<AliasDecl>> aliases;

  Rule(const std::string &name_, const location &loc_);

  Rule *clone() const override = 0;

  virtual std::vector<Ptr<Rule>> flatten() const;

  virtual ~Rule() = default;
};

struct RUMUR_API_WITH_RTTI AliasRule : public Rule {
  std::vector<Ptr<Rule>> rules;

  AliasRule(const std::vector<Ptr<AliasDecl>> &aliases_,
            const std::vector<Ptr<Rule>> &rules_, const location &loc_);
  virtual ~AliasRule() = default;
  AliasRule *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  std::vector<Ptr<Rule>> flatten() const final;
};

struct RUMUR_API_WITH_RTTI SimpleRule : public Rule {

  Ptr<Expr> guard;
  std::vector<Ptr<Decl>> decls;
  std::vector<Ptr<Stmt>> body;

  SimpleRule(const std::string &name_, const Ptr<Expr> &guard_,
             const std::vector<Ptr<Decl>> &decls_,
             const std::vector<Ptr<Stmt>> &body_, const location &loc_);
  virtual ~SimpleRule() = default;
  SimpleRule *clone() const override;
  void validate() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;
};

struct RUMUR_API_WITH_RTTI StartState : public Rule {

  std::vector<Ptr<Decl>> decls;
  std::vector<Ptr<Stmt>> body;

  StartState(const std::string &name_, const std::vector<Ptr<Decl>> &decls_,
             const std::vector<Ptr<Stmt>> &body_, const location &loc_);
  virtual ~StartState() = default;
  StartState *clone() const final;
  void validate() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;
};

struct RUMUR_API_WITH_RTTI PropertyRule : public Rule {

  Property property;

  PropertyRule(const std::string &name_, const Property &property_,
               const location &loc_);
  virtual ~PropertyRule() = default;
  PropertyRule *clone() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;
};

struct RUMUR_API_WITH_RTTI Ruleset : public Rule {

  std::vector<Ptr<Rule>> rules;

  Ruleset(const std::vector<Quantifier> &quantifiers_,
          const std::vector<Ptr<Rule>> &rules_, const location &loc_);
  virtual ~Ruleset() = default;
  Ruleset *clone() const final;
  void validate() const final;

  void visit(BaseTraversal &visitor) final;
  void visit(ConstBaseTraversal &visitor) const final;

  std::vector<Ptr<Rule>> flatten() const final;
};

} // namespace rumur
