#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <rumur/Decl.h>
#include <rumur/except.h>
#include <rumur/Expr.h>
#include <rumur/Function.h>
#include <rumur/Property.h>
#include <rumur/Ptr.h>
#include <rumur/Stmt.h>
#include <rumur/TypeExpr.h>
#include <string>
#include <utility>
#include "utils.h"
#include <vector>

namespace rumur {

Stmt::Stmt(const location &loc_): Node(loc_) { }

AliasStmt::AliasStmt(const std::vector<Ptr<AliasDecl>> &aliases_,
  const std::vector<Ptr<Stmt>> &body_, const location &loc_):
  Stmt(loc_), aliases(aliases_), body(body_) { }

AliasStmt *AliasStmt::clone() const {
  return new AliasStmt(*this);
}

PropertyStmt::PropertyStmt(const Property &property_,
  const std::string &message_, const location &loc_):
  Stmt(loc_), property(property_), message(message_) { }

PropertyStmt *PropertyStmt::clone() const {
  return new PropertyStmt(*this);
}

void PropertyStmt::validate() const {
  if (property.category == Property::LIVENESS)
    throw Error("liveness property appearing as a statement instead of a top "
      "level property", loc);
}

Assignment::Assignment(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
  const location &loc_):
  Stmt(loc_), lhs(lhs_), rhs(rhs_) { }

Assignment *Assignment::clone() const {
  return new Assignment(*this);
}

void Assignment::validate() const {

  if (!lhs->is_lvalue())
    throw Error("non-lvalue expression cannot be assigned to", loc);

  if (lhs->is_readonly())
    throw Error("read-only expression cannot be assigned to", loc);

  if (!lhs->type()->coerces_to(*rhs->type()))
    throw Error("invalid assignment from incompatible type", loc);
}

Clear::Clear(const Ptr<Expr> &rhs_, const location &loc_):
  Stmt(loc_), rhs(rhs_) { }

Clear *Clear::clone() const {
  return new Clear(*this);
}

void Clear::validate() const {
  if (!rhs->is_lvalue())
    throw Error("invalid clear of non-lvalue expression", loc);

  if (rhs->is_readonly())
    throw Error("invalid clear of read-only expression", loc);
}

ErrorStmt::ErrorStmt(const std::string &message_, const location &loc_):
  Stmt(loc_), message(message_) { }

ErrorStmt *ErrorStmt::clone() const {
  return new ErrorStmt(*this);
}

For::For(const Quantifier &quantifier_,
  const std::vector<Ptr<Stmt>> &body_, const location &loc_):
  Stmt(loc_), quantifier(quantifier_), body(body_) { }

For *For::clone() const {
  return new For(*this);
}

IfClause::IfClause(const Ptr<Expr> &condition_,
  const std::vector<Ptr<Stmt>> &body_, const location &loc_):
  Node(loc_), condition(condition_), body(body_) { }

IfClause *IfClause::clone() const {
  return new IfClause(*this);
}

void IfClause::validate() const {

  if (condition == nullptr) {
    // This is an 'else' block.
    return;
  }

  // Only boolean conditions are supported.
  if (!condition->is_boolean())
    throw Error("condition of if clause is not a boolean expression", loc);
}

If::If(const std::vector<IfClause> &clauses_, const location &loc_):
  Stmt(loc_), clauses(clauses_) { }

If *If::clone() const {
  return new If(*this);
}

ProcedureCall::ProcedureCall(const std::string &name,
  const std::vector<Ptr<Expr>> &arguments, const location &loc_):
  Stmt(loc_), call(name, arguments, loc_) {

  call.within_procedure_call = true;
}

ProcedureCall *ProcedureCall::clone() const {
  return new ProcedureCall(*this);
}

Put::Put(const std::string &value_, const location &loc_):
  Stmt(loc_), value(value_) { }

Put::Put(const Ptr<Expr> &expr_, const location &loc_):
  Stmt(loc_), expr(expr_) { }

Put *Put::clone() const {
  return new Put(*this);
}

void Put::validate() const {
  if (expr != nullptr && !expr->is_lvalue() && !expr->type()->is_simple())
    throw Error("printing a complex non-lvalue is not supported", loc);
}

Return::Return(const Ptr<Expr> &expr_, const location &loc_):
  Stmt(loc_), expr(expr_) { }

Return *Return::clone() const {
  return new Return(*this);
}

SwitchCase::SwitchCase(const std::vector<Ptr<Expr>> &matches_,
    const std::vector<Ptr<Stmt>> &body_, const location &loc_):
  Node(loc_), matches(matches_), body(body_) { }

SwitchCase *SwitchCase::clone() const {
  return new SwitchCase(*this);
}

Switch::Switch(const Ptr<Expr> &expr_, const std::vector<SwitchCase> &cases_,
    const location &loc_):
  Stmt(loc_), expr(expr_), cases(cases_) { }

Switch *Switch::clone() const {
  return new Switch(*this);
}

void Switch::validate() const {

  const Ptr<TypeExpr> t = expr->type();
  if (!t->is_simple())
    throw Error("switch expression has complex type", expr->loc);

  for (const SwitchCase &c : cases) {
    for (const Ptr<Expr> &e : c.matches) {
      if (!t->coerces_to(*e->type()))
        throw Error("expression in case cannot be compared to switch "
          "expression", e->loc);
    }
  }
}

Undefine::Undefine(const Ptr<Expr> &rhs_, const location &loc_):
  Stmt(loc_), rhs(rhs_) { }

Undefine *Undefine::clone() const {
  return new Undefine(*this);
}

void Undefine::validate() const {
  if (!rhs->is_lvalue())
    throw Error("invalid undefine of non-lvalue expression", loc);

  if (rhs->is_readonly())
    throw Error("invalid undefine of read-only expression", loc);
}

While::While(const Ptr<Expr> &condition_, const std::vector<Ptr<Stmt>> &body_,
    const location &loc_):
  Stmt(loc_), condition(condition_), body(body_) { }

While *While::clone() const {
  return new While(*this);
}

void While::validate() const {
  if (!condition->is_boolean())
    throw Error("condition in while loop is not a boolean expression",
      condition->loc);
}

}
