#include <rumur/ASTRewriter.h>
#include <rumur/Decl.h>
#include <rumur/Expr.h>
#include <rumur/except.h>
#include <rumur/Function.h>
#include <rumur/Model.h>
#include <rumur/Node.h>
#include <rumur/Property.h>
#include <rumur/Rule.h>
#include <rumur/Stmt.h>
#include <rumur/traverse.h>
#include <rumur/TypeExpr.h>

namespace rumur {

Node *ASTRewriter::visit_add(Add &n) {
  return visit_bexpr(n);
}

Decl *ASTRewriter::visit_aliasdecl(AliasDecl &n) {

  auto value = dynamic_cast<Expr*>(dispatch(*n.value));
  if (value == nullptr) {
    throw Error("invalid deletion of alias value", n.loc);
  }
  if (value != n.value) {
    n.value = value;
  }

  return &n;
}

Rule *ASTRewriter::visit_aliasrule(AliasRule &n) {

  for (size_t i = 0; i < n.aliases.size();) {
    auto decl = dynamic_cast<Decl*>(dispatch(*n.aliases[i]));
    if (decl == nullptr) {
      n.aliases.erase(i);
    } else {
      auto alias = dynamic_cast<AliasDecl*>(decl);
      if (alias == nullptr) {
        throw Error("invalid replacement of alias with a different type of "
          "declaration", n.loc);
      }
      if (alias != n.aliase[i]) {
        n.aliases[i] = alias;
      }
      i++;
    }
  }

  for (size_t i = 0; i < n.rules.size();) {
    auto rule = dynamic_cast<Rule*>(dispatch(*n.rules[i]));
    if (rule == nullptr) {
      n.rules.erase(i);
    } else {
      n.rules[i] = rule;
      i++;
    }
  }

  return &n;
}

Node *ASTRewriter::visit_aliasstmt(AliasStmt &n) {
  for (auto &a : n.aliases)
    dispatch(*a);
  for (auto &s : n.body)
    dispatch(*s);
}

Node *ASTRewriter::visit_and(And &n) {
  visit_bexpr(n);
}

Node *ASTRewriter::visit_array(Array &n) {
  dispatch(*n.index_type);
  dispatch(*n.element_type);
}

Node *ASTRewriter::visit_assignment(Assignment &n) {
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

Node *ASTRewriter::visit_bexpr(BinaryExpr &n) {

  auto lhs = dynamic_cast<Expr*>(dispatch(*n.lhs));
  if (lhs == nullptr) {
    throw Error("invalid deletion of LHS of a binary expression", n.loc);
  }
  if (lhs != n.lhs.get()) {
    n.lhs = lhs;
  }

  auto rhs = dynamic_cast<Expr*>(dispatch(*n.rhs));
  if (rhs == nullptr) {
    throw Error("invalid deletion of RHS of a binary expression", n.loc);
  }
  if (rhs != n.rhs.get()) {
    n.rhs = rhs;
  }

  return &n;
}

  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

Node *ASTRewriter::visit_clear(Clear &n) {
  dispatch(*n.rhs);
}

Node *ASTRewriter::visit_constdecl(ConstDecl &n) {
  dispatch(*n.value);
}

Node *ASTRewriter::visit_div(Div &n) {
  visit_bexpr(n);
}

Node *ASTRewriter::visit_element(Element &n) {
  dispatch(*n.array);
  dispatch(*n.index);
}

Node *ASTRewriter::visit_enum(Enum&) { }

Node *ASTRewriter::visit_eq(Eq &n) {
  visit_bexpr(n);
}

Node *ASTRewriter::visit_errorstmt(ErrorStmt&) { }

Node *ASTRewriter::visit_exists(Exists &n) {
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

Node *ASTRewriter::visit_exprid(ExprID&) { }

Node *ASTRewriter::visit_field(Field &n) {
  dispatch(*n.record);
}

Node *ASTRewriter::visit_for(For &n) {
  dispatch(n.quantifier);
  for (auto &s : n.body)
    dispatch(*s);
}

Node *ASTRewriter::visit_forall(Forall &n) {
  dispatch(n.quantifier);
  dispatch(*n.expr);
}

Node *ASTRewriter::visit_function(Function &n) {
  for (auto &p : n.parameters)
    dispatch(*p);
  if (n.return_type != nullptr)
    dispatch(*n.return_type);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

Node *ASTRewriter::visit_functioncall(FunctionCall &n) {
  for (auto &a : n.arguments)
    dispatch(*a);
}

Node *ASTRewriter::visit_geq(Geq &n) {
  visit_bexpr(n);
}

Node *ASTRewriter::visit_gt(Gt &n) {
  visit_bexpr(n);
}

Node *ASTRewriter::visit_if(If &n) {
  for (IfClause &c : n.clauses)
    dispatch(c);
}

Node *ASTRewriter::visit_ifclause(IfClause &n) {
  if (n.condition != nullptr)
    dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

Node *ASTRewriter::visit_implication(Implication &n) {
  visit_bexpr(n);
}

Node *ASTRewriter::visit_isundefined(IsUndefined &n) {
  dispatch(*n.expr);
}

Node *ASTRewriter::visit_leq(Leq &n) {
  visit_bexpr(n);
}

Node *ASTRewriter::visit_lt(Lt &n) {
  visit_bexpr(n);
}

Node *ASTRewriter::visit_mod(Mod &n) {
  visit_bexpr(n);
}

Node *ASTRewriter::visit_model(Model &n) {
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &f : n.functions)
    dispatch(*f);
  for (auto &r : n.rules)
    dispatch(*r);
}

Node *ASTRewriter::visit_mul(Mul &n) {
  visit_bexpr(n);
}

Node *ASTRewriter::visit_negative(Negative &n) {
  visit_uexpr(n);
}

Node *ASTRewriter::visit_neq(Neq &n) {
  visit_bexpr(n);
}

Node *ASTRewriter::visit_not(Not &n) {
  visit_uexpr(n);
}

Node *ASTRewriter::visit_number(Number&) { }

Node *ASTRewriter::visit_or(Or &n) {
  visit_bexpr(n);
}

Node *ASTRewriter::visit_procedurecall(ProcedureCall &n) {
  dispatch(n.call);
}

Node *ASTRewriter::visit_property(Property &n) {
  dispatch(*n.expr);
}

Node *ASTRewriter::visit_propertyrule(PropertyRule &n) {
  for (Quantifier &q : n.quantifiers)
    dispatch(q);
  dispatch(n.property);
}

Node *ASTRewriter::visit_propertystmt(PropertyStmt &n) {
  dispatch(n.property);
}

Node *ASTRewriter::visit_put(Put &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

Node *ASTRewriter::visit_quantifier(Quantifier &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
  if (n.from != nullptr)
    dispatch(*n.from);
  if (n.to != nullptr)
    dispatch(*n.to);
  if (n.step != nullptr)
    dispatch(*n.step);
}

Node *ASTRewriter::visit_range(Range &n) {
  dispatch(*n.min);
  dispatch(*n.max);
}

Node *ASTRewriter::visit_record(Record &n) {
  for (auto &f : n.fields)
    dispatch(*f);
}

Node *ASTRewriter::visit_return(Return &n) {
  if (n.expr != nullptr)
    dispatch(*n.expr);
}

Node *ASTRewriter::visit_ruleset(Ruleset &n) {
  for (Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &r : n.rules)
    dispatch(*r);
}

Node *ASTRewriter::visit_scalarset(Scalarset &n) {
  dispatch(*n.bound);
}

Node *ASTRewriter::visit_simplerule(SimpleRule &n) {
  for (Quantifier &q : n.quantifiers)
    dispatch(q);
  if (n.guard != nullptr)
    dispatch(*n.guard);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

Node *ASTRewriter::visit_startstate(StartState &n) {
  for (Quantifier &q : n.quantifiers)
    dispatch(q);
  for (auto &d : n.decls)
    dispatch(*d);
  for (auto &s : n.body)
    dispatch(*s);
}

Node *ASTRewriter::visit_sub(Sub &n) {
  visit_bexpr(n);
}

Node *ASTRewriter::visit_switch(Switch &n) {
  dispatch(*n.expr);
  for (SwitchCase &c : n.cases)
    dispatch(c);
}

Node *ASTRewriter::visit_switchcase(SwitchCase &n) {
  for (auto &m : n.matches)
    dispatch(*m);
  for (auto &s : n.body)
    dispatch(*s);
}

Node *ASTRewriter::visit_ternary(Ternary &n) {
  dispatch(*n.cond);
  dispatch(*n.lhs);
  dispatch(*n.rhs);
}

Node *ASTRewriter::visit_typedecl(TypeDecl &n) {
  dispatch(*n.value);
}

Node *ASTRewriter::visit_typeexprid(TypeExprID&) { }

Node *ASTRewriter::visit_uexpr(UnaryExpr &n) {
  dispatch(*n.rhs);
}

Node *ASTRewriter::visit_undefine(Undefine &n) {
  dispatch(*n.rhs);
}

Node *ASTRewriter::visit_vardecl(VarDecl &n) {
  if (n.type != nullptr)
    dispatch(*n.type);
}

Node *ASTRewriter::visit_while(While &n) {
  dispatch(*n.condition);
  for (auto &s : n.body)
    dispatch(*s);
}

Traversal::~Traversal() { }

}
