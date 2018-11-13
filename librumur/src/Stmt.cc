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

AliasStmt::AliasStmt(std::vector<std::shared_ptr<AliasDecl>> &&aliases_,
  const std::vector<Ptr<Stmt>> &body_, const location &loc_):
  Stmt(loc_), aliases(aliases_), body(body_) { }

AliasStmt::AliasStmt(const AliasStmt &other):
  Stmt(other.loc), body(other.body) {

  for (const std::shared_ptr<AliasDecl> &a : other.aliases)
    aliases.emplace_back(a->clone());
}

AliasStmt &AliasStmt::operator=(AliasStmt other) {
  swap(*this, other);
  return *this;
}

void swap(AliasStmt &x, AliasStmt &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.aliases, y.aliases);
  swap(x.body, y.body);
}

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

Assignment::Assignment(std::shared_ptr<Expr> lhs_, std::shared_ptr<Expr> rhs_,
  const location &loc_):
  Stmt(loc_), lhs(lhs_), rhs(rhs_) { }

Assignment::Assignment(const Assignment &other):
  Stmt(other), lhs(other.lhs->clone()), rhs(other.rhs->clone()) {
}

Assignment &Assignment::operator=(Assignment other) {
  swap(*this, other);
  return *this;
}

void swap(Assignment &x, Assignment &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.lhs, y.lhs);
  swap(x.rhs, y.rhs);
}

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

Clear::Clear(std::shared_ptr<Expr> rhs_, const location &loc_):
  Stmt(loc_), rhs(rhs_) { }

Clear::Clear(const Clear &other):
  Stmt(other.loc), rhs(other.rhs->clone()) { }

Clear &Clear::operator=(Clear other) {
  swap(*this, other);
  return *this;
}

void swap(Clear &x, Clear &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.rhs, y.rhs);
}

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

For::For(const For &other):
  Stmt(other.loc), quantifier(other.quantifier), body(other.body) { }

For &For::operator=(For other) {
  swap(*this, other);
  return *this;
}

void swap(For &x, For &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.quantifier, y.quantifier);
  swap(x.body, y.body);
}

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

IfClause::IfClause(std::shared_ptr<Expr> condition_,
  const std::vector<Ptr<Stmt>> &body_, const location &loc_):
  Node(loc_), condition(condition_), body(body_) { }

IfClause::IfClause(const IfClause &other):
  Node(other.loc),
  condition(other.condition == nullptr ? nullptr : other.condition->clone()),
  body(other.body) {
}

IfClause &IfClause::operator=(IfClause other) {
  swap(*this, other);
  return *this;
}

void swap(IfClause &x, IfClause &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.condition, y.condition);
  swap(x.body, y.body);
}

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

If::If(std::vector<IfClause> &&clauses_, const location &loc_):
  Stmt(loc_), clauses(clauses_) { }

If::If(const If &other):
  Stmt(other.loc), clauses(other.clauses) { }

If &If::operator=(If other) {
  swap(*this, other);
  return *this;
}

void swap(If &x, If &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.clauses, y.clauses);
}

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

ProcedureCall::ProcedureCall(const std::string &name_, std::shared_ptr<Function> function_,
  std::vector<std::shared_ptr<Expr>> &&arguments_, const location &loc_):
  Stmt(loc_), name(name_), function(function_), arguments(arguments_) { }

ProcedureCall::ProcedureCall(const ProcedureCall &other):
  Stmt(other.loc), name(other.name),
  function(other.function == nullptr ? nullptr : other.function->clone()) {

  for (const std::shared_ptr<Expr> &p : other.arguments)
    arguments.emplace_back(p->clone());
}

ProcedureCall &ProcedureCall::operator=(ProcedureCall other) {
  swap(*this, other);
  return *this;
}

void swap(ProcedureCall &x, ProcedureCall &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.name, y.name);
  swap(x.function, y.function);
  swap(x.arguments, y.arguments);
}

ProcedureCall *ProcedureCall::clone() const {
  return new ProcedureCall(*this);
}

bool ProcedureCall::operator==(const Node &other) const {
  auto o = dynamic_cast<const ProcedureCall*>(&other);
  if (o == nullptr)
    return false;
  if (name != o->name)
    return false;
  if (function == nullptr) {
    if (o->function != nullptr)
      return false;
  } else if (o->function == nullptr) {
    return false;
  } else if (*function != *o->function) {
    return false;
  }
  if (!vector_eq(arguments, o->arguments))
    return false;
  return true;
}

void ProcedureCall::validate() const {
  if (function == nullptr)
    throw Error("unknown procedure \"" + name + "\"", loc);
}

Return::Return(std::shared_ptr<Expr> expr_, const location &loc_):
  Stmt(loc_), expr(expr_) { }

Return::Return(const Return &other):
  Stmt(other.loc), expr(other.expr == nullptr ? nullptr : other.expr->clone()) { }

Return &Return::operator=(Return other) {
  swap(*this, other);
  return *this;
}

void swap(Return &x, Return &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.expr, y.expr);
}

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

Undefine::Undefine(std::shared_ptr<Expr> rhs_, const location &loc_):
  Stmt(loc_), rhs(rhs_) { }

Undefine::Undefine(const Undefine &other):
  Stmt(other.loc), rhs(other.rhs->clone()) { }

Undefine &Undefine::operator=(Undefine other) {
  swap(*this, other);
  return *this;
}

void swap(Undefine &x, Undefine &y) noexcept {
  using std::swap;
  swap(x.loc, y.loc);
  swap(x.unique_id, y.unique_id);
  swap(x.rhs, y.rhs);
}

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

}
