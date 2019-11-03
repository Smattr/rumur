#pragma once

#include <climits>
#include <iostream>
#include <rumur/rumur.h>
#include <string>
#include <sstream>
#include <vector>

class Printer : public rumur::ConstBaseTraversal {

 private:
  std::istream &in;
  std::ostream &out;

  // current position within the input file
  unsigned long line = 1;
  unsigned long column = 1;

  // does the next semantically relevant output character need to be a
  // semicolon?
  bool pending_semi = false;

  // buffered content that we have notionally written but may need to add
  // characters before it in the output
  std::ostringstream pending;

  // ranges from the source file that should be dropped in the output file
  std::vector<rumur::location> deletions;

 public:
  Printer(std::istream &in_, std::ostream &out_,
    const std::vector<rumur::location> &deletions_);

  void visit_add(const rumur::Add &n) final;
  void visit_aliasdecl(const rumur::AliasDecl &n) final;
  void visit_aliasrule(const rumur::AliasRule &n) final;
  void visit_aliasstmt(const rumur::AliasStmt &n) final;
  void visit_and(const rumur::And &n) final;
  void visit_array(const rumur::Array &n) final;
  void visit_assignment(const rumur::Assignment &n) final;
  void visit_clear(const rumur::Clear &n) final;
  void visit_constdecl(const rumur::ConstDecl &n) final;
  void visit_div(const rumur::Div &n) final;
  void visit_element(const rumur::Element &n) final;
  void visit_enum(const rumur::Enum &n) final;
  void visit_eq(const rumur::Eq &n) final;
  void visit_errorstmt(const rumur::ErrorStmt &n) final;
  void visit_exists(const rumur::Exists &n) final;
  void visit_exprid(const rumur::ExprID &n) final;
  void visit_field(const rumur::Field &n) final;
  void visit_for(const rumur::For &n) final;
  void visit_forall(const rumur::Forall &n) final;
  void visit_function(const rumur::Function &n) final;
  void visit_functioncall(const rumur::FunctionCall &n) final;
  void visit_geq(const rumur::Geq &n) final;
  void visit_gt(const rumur::Gt &n) final;
  void visit_if(const rumur::If &n) final;
  void visit_ifclause(const rumur::IfClause &n) final;
  void visit_implication(const rumur::Implication &n) final;
  void visit_isundefined(const rumur::IsUndefined &n) final;
  void visit_leq(const rumur::Leq &n) final;
  void visit_lt(const rumur::Lt &n) final;
  void visit_mod(const rumur::Mod &n) final;
  void visit_model(const rumur::Model &n) final;
  void visit_mul(const rumur::Mul &n) final;
  void visit_negative(const rumur::Negative &n) final;
  void visit_neq(const rumur::Neq &n) final;
  void visit_not(const rumur::Not &n) final;
  void visit_number(const rumur::Number &n) final;
  void visit_or(const rumur::Or &n) final;
  void visit_procedurecall(const rumur::ProcedureCall &n) final;
  void visit_property(const rumur::Property &n) final;
  void visit_propertyrule(const rumur::PropertyRule &n) final;
  void visit_propertystmt(const rumur::PropertyStmt &n) final;
  void visit_put(const rumur::Put &n) final;
  void visit_quantifier(const rumur::Quantifier &n) final;
  void visit_range(const rumur::Range &n) final;
  void visit_record(const rumur::Record &n) final;
  void visit_return(const rumur::Return &n) final;
  void visit_ruleset(const rumur::Ruleset &n) final;
  void visit_scalarset(const rumur::Scalarset &n) final;
  void visit_simplerule(const rumur::SimpleRule &n) final;
  void visit_startstate(const rumur::StartState &n) final;
  void visit_sub(const rumur::Sub &n) final;
  void visit_switch(const rumur::Switch &n) final;
  void visit_switchcase(const rumur::SwitchCase &n) final;
  void visit_ternary(const rumur::Ternary &n) final;
  void visit_typedecl(const rumur::TypeDecl &n) final;
  void visit_typeexprid(const rumur::TypeExprID &n) final;
  void visit_undefine(const rumur::Undefine &n) final;
  void visit_vardecl(const rumur::VarDecl &n) final;
  void visit_while(const rumur::While &n) final;

  virtual ~Printer();

 private:
  void visit_bexpr(const std::string &tag, const rumur::BinaryExpr &n);
  void visit_uexpr(const std::string &tag, const rumur::UnaryExpr &n);
  void sync_to(const rumur::Node &n);
  void sync_to(const rumur::position &pos
    = rumur::position(nullptr, UINT_MAX, UINT_MAX));

  // should this character from the input be dropped in the output?
  bool deleted(const rumur::position &pos) const;
};
