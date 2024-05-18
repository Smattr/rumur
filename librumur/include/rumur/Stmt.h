#pragma once

#include "location.hh"
#include <cstddef>
#include <iostream>
#include <memory>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/Node.h>
#include <rumur/Property.h>
#include <rumur/Ptr.h>
#include <string>
#include <vector>

#ifndef RUMUR_API_WITH_RTTI
#define RUMUR_API_WITH_RTTI __attribute__((visibility("default")))
#endif

namespace rumur {

struct RUMUR_API_WITH_RTTI Stmt : public Node {

  Stmt(const location &loc_);

  virtual ~Stmt() = default;
  virtual Stmt *clone() const = 0;

protected:
  Stmt(const Stmt &) = default;
  Stmt &operator=(const Stmt &) = default;
};

struct RUMUR_API_WITH_RTTI AliasStmt : public Stmt {

  std::vector<Ptr<AliasDecl>> aliases;
  std::vector<Ptr<Stmt>> body;

  AliasStmt(const std::vector<Ptr<AliasDecl>> &aliases_,
            const std::vector<Ptr<Stmt>> &body_, const location &loc_);
  AliasStmt *clone() const override;
  virtual ~AliasStmt() = default;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;
};

struct RUMUR_API_WITH_RTTI PropertyStmt : public Stmt {

  Property property;
  std::string message;

  PropertyStmt(const Property &property_, const std::string &message_,
               const location &loc_);
  PropertyStmt *clone() const override;
  virtual ~PropertyStmt() = default;

  void validate() const override;
  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;
};

struct RUMUR_API_WITH_RTTI Assignment : public Stmt {

  Ptr<Expr> lhs;
  Ptr<Expr> rhs;

  Assignment(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
             const location &loc_);
  Assignment *clone() const override;
  virtual ~Assignment() = default;

  void validate() const override;
  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;
};

struct RUMUR_API_WITH_RTTI Clear : public Stmt {

  Ptr<Expr> rhs;

  Clear(const Ptr<Expr> &rhs_, const location &loc);
  virtual ~Clear() = default;
  Clear *clone() const override;

  void validate() const override;
  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;
};

struct RUMUR_API_WITH_RTTI ErrorStmt : public Stmt {

  std::string message;

  ErrorStmt(const std::string &message_, const location &loc_);
  ErrorStmt *clone() const override;
  virtual ~ErrorStmt() = default;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;
};

struct RUMUR_API_WITH_RTTI For : public Stmt {

  Quantifier quantifier;
  std::vector<Ptr<Stmt>> body;

  For(const Quantifier &quantifier_, const std::vector<Ptr<Stmt>> &body_,
      const location &loc_);
  virtual ~For() = default;
  For *clone() const override;

  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;
};

struct RUMUR_API_WITH_RTTI IfClause : public Node {

  Ptr<Expr> condition;
  std::vector<Ptr<Stmt>> body;

  IfClause(const Ptr<Expr> &condition_, const std::vector<Ptr<Stmt>> &body_,
           const location &loc_);
  virtual ~IfClause() = default;
  IfClause *clone() const override;

  void validate() const override;
  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;
};

struct RUMUR_API_WITH_RTTI If : public Stmt {

  std::vector<IfClause> clauses;

  If(const std::vector<IfClause> &clauses_, const location &loc_);
  virtual ~If() = default;
  If *clone() const override;
  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;
};

struct RUMUR_API_WITH_RTTI ProcedureCall : public Stmt {

  FunctionCall call;

  ProcedureCall(const std::string &name,
                const std::vector<Ptr<Expr>> &arguments, const location &loc_);
  virtual ~ProcedureCall() = default;
  ProcedureCall *clone() const override;
  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;
};

struct RUMUR_API_WITH_RTTI Put : public Stmt {

  std::string value;
  Ptr<Expr> expr;

  Put(const std::string &value_, const location &loc_);
  Put(const Ptr<Expr> &expr_, const location &loc_);
  virtual ~Put() = default;
  Put *clone() const override;

  void validate() const override;
  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;
};

struct RUMUR_API_WITH_RTTI Return : public Stmt {

  Ptr<Expr> expr;

  Return(const Ptr<Expr> &expr_, const location &loc_);
  virtual ~Return() = default;
  Return *clone() const override;
  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;
};

struct RUMUR_API_WITH_RTTI SwitchCase : public Node {

  std::vector<Ptr<Expr>> matches;
  std::vector<Ptr<Stmt>> body;

  SwitchCase(const std::vector<Ptr<Expr>> &matches_,
             const std::vector<Ptr<Stmt>> &body_, const location &loc_);
  virtual ~SwitchCase() = default;
  SwitchCase *clone() const override;
  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;
};

struct RUMUR_API_WITH_RTTI Switch : public Stmt {

  Ptr<Expr> expr;
  std::vector<SwitchCase> cases;

  Switch(const Ptr<Expr> &expr_, const std::vector<SwitchCase> &cases_,
         const location &loc_);
  virtual ~Switch() = default;
  Switch *clone() const override;

  void validate() const override;
  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;
};

struct RUMUR_API_WITH_RTTI Undefine : public Stmt {

  Ptr<Expr> rhs;

  Undefine(const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Undefine() = default;
  Undefine *clone() const override;

  void validate() const override;
  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;
};

struct RUMUR_API_WITH_RTTI While : public Stmt {

  Ptr<Expr> condition;
  std::vector<Ptr<Stmt>> body;

  While(const Ptr<Expr> &condition_, const std::vector<Ptr<Stmt>> &body_,
        const location &loc_);
  virtual ~While() = default;
  While *clone() const override;

  void validate() const override;
  void visit(BaseTraversal &visitor) override;
  void visit(ConstBaseTraversal &visitor) const override;
};

} // namespace rumur
