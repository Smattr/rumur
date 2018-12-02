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
#include <string>
#include <vector>

namespace rumur {

// Forward declare a struct we need to point to, to avoid a circular #include
struct Function;

struct Stmt : public Node {

  using Node::Node;

  virtual ~Stmt() = default;
  virtual Stmt *clone() const = 0;

};

struct AliasStmt : public Stmt {

  std::vector<Ptr<AliasDecl>> aliases;
  std::vector<Ptr<Stmt>> body;

  AliasStmt(const std::vector<Ptr<AliasDecl>> &aliases_,
    const std::vector<Ptr<Stmt>> &body_, const location &loc_);
  AliasStmt *clone() const final;
  virtual ~AliasStmt() = default;

  bool operator==(const Node &other) const final;
};

struct PropertyStmt : public Stmt {

  Property property;
  std::string message;

  PropertyStmt(const Property &property_, const std::string &message_,
    const location &loc_);
  PropertyStmt *clone() const final;
  virtual ~PropertyStmt() = default;

  bool operator==(const Node &other) const final;
};

struct Assignment : public Stmt {

  Ptr<Expr> lhs;
  Ptr<Expr> rhs;

  Assignment(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
    const location &loc_);
  Assignment *clone() const final;
  virtual ~Assignment() = default;

  bool operator==(const Node &other) const final;
  void validate() const final;
};

struct Clear : public Stmt {

  Ptr<Expr> rhs;

  Clear(const Ptr<Expr> &rhs_, const location &loc);
  virtual ~Clear() = default;
  Clear *clone() const final;

  bool operator==(const Node &other) const final;
  void validate() const final;
};

struct ErrorStmt : public Stmt {

   std::string message;

   ErrorStmt(const std::string &message_, const location &loc_);
   ErrorStmt *clone() const final;
   virtual ~ErrorStmt() = default;

   bool operator==(const Node &other) const final;
};

struct For : public Stmt {

  Quantifier quantifier;
  std::vector<Ptr<Stmt>> body;

  For(const Quantifier &quantifier_,
    const std::vector<Ptr<Stmt>> &body_, const location &loc_);
  virtual ~For() = default;
  For *clone() const final;

  bool operator==(const Node &other) const final;
};

struct IfClause : public Node {

  Ptr<Expr> condition;
  std::vector<Ptr<Stmt>> body;

  IfClause(const Ptr<Expr> &condition_,
    const std::vector<Ptr<Stmt>> &body_, const location &loc_);
  virtual ~IfClause() = default;
  IfClause *clone() const final;

  bool operator==(const Node &other) const final;
  void validate() const final;
};

struct If : public Stmt {

  std::vector<IfClause> clauses;

  If(const std::vector<IfClause> &clauses_, const location &loc_);
  virtual ~If() = default;;
  If *clone() const final;

  bool operator==(const Node &other) const final;
};

struct ProcedureCall : public Stmt {

  std::string name;
  Ptr<Function> function;
  std::vector<Ptr<Expr>> arguments;

  ProcedureCall(const std::string &name_,
    const std::vector<Ptr<Expr>> &arguments_, const location &loc_);
  virtual ~ProcedureCall() = default;
  ProcedureCall *clone() const final;

  bool operator==(const Node &other) const final;
  void validate() const final;
};

struct Put : public Stmt {

  std::string value;
  Ptr<Expr> expr;

  Put(const std::string &value_, const location &loc_);
  Put(const Ptr<Expr> &expr_, const location &loc_);
  virtual ~Put() = default;
  Put *clone() const final;

  bool operator==(const Node &other) const final;
  void validate() const final;
};

struct Return : public Stmt {

  Ptr<Expr> expr;

  Return(const Ptr<Expr> &expr_, const location &loc_);
  virtual ~Return() = default;
  Return *clone() const final;

  bool operator==(const Node &other) const final;
};

struct SwitchCase : public Node {

  std::vector<Ptr<Expr>> matches;
  std::vector<Ptr<Stmt>> body;

  SwitchCase(const std::vector<Ptr<Expr>> &matches_,
    const std::vector<Ptr<Stmt>> &body_, const location &loc_);
  virtual ~SwitchCase() = default;
  SwitchCase *clone() const final;

  bool operator==(const Node &other) const final;
};

struct Switch : public Stmt {

  Ptr<Expr> expr;
  std::vector<SwitchCase> cases;

  Switch(const Ptr<Expr> &expr_, const std::vector<SwitchCase> &cases_,
    const location &loc_);
  virtual ~Switch() = default;
  Switch *clone() const final;

  bool operator==(const Node &other) const final;
  void validate() const final;
};

struct Undefine : public Stmt {

  Ptr<Expr> rhs;

  Undefine(const Ptr<Expr> &rhs_, const location &loc_);
  virtual ~Undefine() = default;
  Undefine *clone() const final;

  bool operator==(const Node &other) const final;
  void validate() const final;
};

struct While : public Stmt {

  Ptr<Expr> condition;
  std::vector<Ptr<Stmt>> body;

  While(const Ptr<Expr> &condition_, const std::vector<Ptr<Stmt>> &body_,
    const location &loc_);
  virtual ~While() = default;
  While *clone() const final;

  bool operator==(const Node &other) const final;
  void validate() const final;
};

}
