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

AliasStmt::AliasStmt(const std::vector<Ptr<AliasDecl>> &aliases_,
  const std::vector<Ptr<Stmt>> &body_, const location &loc_):
  Stmt(loc_), aliases(aliases_), body(body_) { }

AliasStmt *AliasStmt::clone() const {
  return new AliasStmt(*this);
}

bool AliasStmt::operator==(const Node &other) const {
  auto o = dynamic_cast<const AliasStmt*>(&other);
  if (o == nullptr)
    return false;
  if (!vector_eq(aliases, o->aliases))
    return false;
  if (!vector_eq(body, o->body))
    return false;
  return true;
}

PropertyStmt::PropertyStmt(const Property &property_,
  const std::string &message_, const location &loc_):
  Stmt(loc_), property(property_), message(message_) { }

PropertyStmt *PropertyStmt::clone() const {
  return new PropertyStmt(*this);
}

bool PropertyStmt::operator==(const Node &other) const {
  auto o = dynamic_cast<const PropertyStmt*>(&other);
  return o != nullptr && property == o->property && message == o->message;
}

Assignment::Assignment(const Ptr<Expr> &lhs_, const Ptr<Expr> &rhs_,
  const location &loc_):
  Stmt(loc_), lhs(lhs_), rhs(rhs_) { }

Assignment *Assignment::clone() const {
  return new Assignment(*this);
}

bool Assignment::operator==(const Node &other) const {
  auto o = dynamic_cast<const Assignment*>(&other);
  return o != nullptr && *lhs == *o->lhs && *rhs == *o->rhs;
}

void Assignment::validate() const {

  if (!lhs->is_lvalue())
    throw Error("non-lvalue expression cannot be assigned to", loc);

  const TypeExpr *lhs_type = lhs->type();
  assert(lhs_type != nullptr && "left hand side of assignment has numeric "
    "literal type");
  lhs_type = lhs_type->resolve();

  const TypeExpr *rhs_type = rhs->type();
  if (rhs_type != nullptr)
    rhs_type = rhs_type->resolve();

  if (isa<Range>(lhs_type) && (rhs_type == nullptr || isa<Range>(rhs_type)))
    return;

  if (rhs_type != nullptr && *lhs_type == *rhs_type)
    return;

  throw Error("invalid assignment from incompatible type", loc);
}

Clear::Clear(const Ptr<Expr> &rhs_, const location &loc_):
  Stmt(loc_), rhs(rhs_) { }

Clear *Clear::clone() const {
  return new Clear(*this);
}

bool Clear::operator==(const Node &other) const {
  auto o = dynamic_cast<const Clear*>(&other);
  if (o == nullptr)
    return false;
  if (*rhs != *o->rhs)
    return false;
  return true;
}

void Clear::validate() const {
  if (!rhs->is_lvalue())
    throw Error("invalid clear of non-lvalue expression", loc);
}

ErrorStmt::ErrorStmt(const std::string &message_, const location &loc_):
  Stmt(loc_), message(message_) { }

ErrorStmt *ErrorStmt::clone() const {
  return new ErrorStmt(*this);
}

bool ErrorStmt::operator==(const Node &other) const {
  auto o = dynamic_cast<const ErrorStmt*>(&other);
  return o != nullptr && message == o->message;
}

For::For(const Quantifier &quantifier_,
  const std::vector<Ptr<Stmt>> &body_, const location &loc_):
  Stmt(loc_), quantifier(quantifier_), body(body_) { }

For *For::clone() const {
  return new For(*this);
}

bool For::operator==(const Node &other) const {
  auto o = dynamic_cast<const For*>(&other);
  if (o == nullptr)
    return false;
  if (quantifier != o->quantifier)
    return false;
  if (!vector_eq(body, o->body))
    return false;
  return true;
}

IfClause::IfClause(const Ptr<Expr> &condition_,
  const std::vector<Ptr<Stmt>> &body_, const location &loc_):
  Node(loc_), condition(condition_), body(body_) { }

IfClause *IfClause::clone() const {
  return new IfClause(*this);
}

bool IfClause::operator==(const Node &other) const {
  auto o = dynamic_cast<const IfClause*>(&other);
  if (o == nullptr)
    return false;
  if (condition == nullptr) {
    if (o->condition != nullptr)
      return false;
  } else if (o->condition == nullptr) {
    return false;
  } else if (*condition != *o->condition) {
    return false;
  }
  if (!vector_eq(body, o->body))
    return false;
  return true;
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

bool If::operator==(const Node &other) const {
  auto o = dynamic_cast<const If*>(&other);
  if (o == nullptr)
    return false;
  if (clauses != o->clauses)
    return false;
  return true;
}

ProcedureCall::ProcedureCall(const std::string &name,
  const std::vector<Ptr<Expr>> &arguments, const location &loc_):
  Stmt(loc_), call(name, arguments, loc_) { }

ProcedureCall *ProcedureCall::clone() const {
  return new ProcedureCall(*this);
}

bool ProcedureCall::operator==(const Node &other) const {
  auto o = dynamic_cast<const ProcedureCall*>(&other);
  if (o == nullptr)
    return false;
  if (call != o->call)
    return false;
  return true;
}

Put::Put(const std::string &value_, const location &loc_):
  Stmt(loc_), value(value_) { }

Put::Put(const Ptr<Expr> &expr_, const location &loc_):
  Stmt(loc_), expr(expr_) { }

Put *Put::clone() const {
  return new Put(*this);
}

bool Put::operator==(const Node &other) const {
  auto o = dynamic_cast<const Put*>(&other);
  if (o == nullptr)
    return false;
  if (value != o->value)
    return false;
  if (expr == nullptr) {
    if (o->expr != nullptr)
      return false;
  } else if (o->expr == nullptr) {
    return false;
  } else if (*expr != *o->expr) {
    return false;
  }
  return true;
}

void Put::validate() const {
  if (expr != nullptr && expr->type() != nullptr && !expr->is_lvalue()
      && !expr->type()->is_simple())
    throw Error("printing a complex non-lvalue is not supported", loc);
}

Return::Return(const Ptr<Expr> &expr_, const location &loc_):
  Stmt(loc_), expr(expr_) { }

Return *Return::clone() const {
  return new Return(*this);
}

bool Return::operator==(const Node &other) const {
  auto o = dynamic_cast<const Return*>(&other);
  if (o == nullptr)
    return false;
  if (expr == nullptr) {
    if (o->expr != nullptr)
      return false;
  } else if (o->expr == nullptr) {
    return false;
  } else if (*expr != *o->expr) {
    return false;
  }
  return true;
}

SwitchCase::SwitchCase(const std::vector<Ptr<Expr>> &matches_,
    const std::vector<Ptr<Stmt>> &body_, const location &loc_):
  Node(loc_), matches(matches_), body(body_) { }

SwitchCase *SwitchCase::clone() const {
  return new SwitchCase(*this);
}

bool SwitchCase::operator==(const Node &other) const {
  auto o = dynamic_cast<const SwitchCase*>(&other);
  if (o == nullptr)
    return false;
  if (!vector_eq(matches, o->matches))
    return false;
  if (!vector_eq(body, o->body))
    return false;
  return true;
}

Switch::Switch(const Ptr<Expr> &expr_, const std::vector<SwitchCase> &cases_,
    const location &loc_):
  Stmt(loc_), expr(expr_), cases(cases_) { }

Switch *Switch::clone() const {
  return new Switch(*this);
}

bool Switch::operator==(const Node &other) const {
  auto o = dynamic_cast<const Switch*>(&other);
  if (o == nullptr)
    return false;
  if (*expr != *o->expr)
    return false;
  if (cases != o->cases)
    return false;
  return true;
}

void Switch::validate() const {

  const TypeExpr *t = expr->type();
  if (t != nullptr && !t->is_simple())
    throw Error("switch expression has complex type", expr->loc);

  for (const SwitchCase &c : cases) {
    for (const Ptr<Expr> &e : c.matches) {
      if (!types_equatable(t, e->type()))
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

bool Undefine::operator==(const Node &other) const {
  auto o = dynamic_cast<const Undefine*>(&other);
  if (o == nullptr)
    return false;
  if (*rhs != *o->rhs)
    return false;
  return true;
}

void Undefine::validate() const {
  if (!rhs->is_lvalue())
    throw Error("invalid undefine of non-lvalue expression", loc);
}

While::While(const Ptr<Expr> &condition_, const std::vector<Ptr<Stmt>> &body_,
    const location &loc_):
  Stmt(loc_), condition(condition_), body(body_) { }

While *While::clone() const {
  return new While(*this);
}

bool While::operator==(const Node &other) const {
  auto o = dynamic_cast<const While*>(&other);
  if (o == nullptr)
    return false;
  if (*condition != *o->condition)
    return false;
  if (!vector_eq(body, o->body))
    return false;
  return true;
}

void While::validate() const {
  if (!condition->is_boolean())
    throw Error("condition in while loop is not a boolean expression",
      condition->loc);
}

}
