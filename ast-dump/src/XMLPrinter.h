#pragma once

#include <climits>
#include <iostream>
#include <rumur/rumur.h>
#include <string>

class XMLPrinter : public rumur::ConstBaseTraversal {

 private:
  std::ostream *o;
  std::istream *in = nullptr;
  unsigned long line = 1;
  unsigned long column = 1;

 public:
  XMLPrinter(const std::string &in_filename, std::ostream &o_);

  void visit(const rumur::Add &n) final;
  void visit(const rumur::AliasDecl &n) final;
  void visit(const rumur::AliasRule &n) final;
  void visit(const rumur::AliasStmt &n) final;
  void visit(const rumur::And &n) final;
  void visit(const rumur::Array &n) final;
  void visit(const rumur::Assignment &n) final;
  void visit(const rumur::Clear &n) final;
  void visit(const rumur::ConstDecl &n) final;
  void visit(const rumur::Div &n) final;
  void visit(const rumur::Element &n) final;
  void visit(const rumur::Enum &n) final;
  void visit(const rumur::Eq &n) final;
  void visit(const rumur::ErrorStmt &n) final;
  void visit(const rumur::Exists &n) final;
  void visit(const rumur::ExprID &n) final;
  void visit(const rumur::Field &n) final;
  void visit(const rumur::For &n) final;
  void visit(const rumur::Forall &n) final;
  void visit(const rumur::Function &n) final;
  void visit(const rumur::FunctionCall &n) final;
  void visit(const rumur::Geq &n) final;
  void visit(const rumur::Gt &n) final;
  void visit(const rumur::If &n) final;
  void visit(const rumur::IfClause &n) final;
  void visit(const rumur::Implication &n) final;
  void visit(const rumur::IsUndefined &n) final;
  void visit(const rumur::Leq &n) final;
  void visit(const rumur::Lt &n) final;
  void visit(const rumur::Mod &n) final;
  void visit(const rumur::Model &n) final;
  void visit(const rumur::Mul &n) final;
  void visit(const rumur::Negative &n) final;
  void visit(const rumur::Neq &n) final;
  void visit(const rumur::Not &n) final;
  void visit(const rumur::Number &n) final;
  void visit(const rumur::Or &n) final;
  void visit(const rumur::ProcedureCall &n) final;
  void visit(const rumur::Property &n) final;
  void visit(const rumur::PropertyRule &n) final;
  void visit(const rumur::PropertyStmt &n) final;
  void visit(const rumur::Put &n) final;
  void visit(const rumur::Quantifier &n) final;
  void visit(const rumur::Range &n) final;
  void visit(const rumur::Record &n) final;
  void visit(const rumur::Return &n) final;
  void visit(const rumur::Ruleset &n) final;
  void visit(const rumur::Scalarset &n) final;
  void visit(const rumur::SimpleRule &n) final;
  void visit(const rumur::StartState &n) final;
  void visit(const rumur::Sub &n) final;
  void visit(const rumur::Switch &n) final;
  void visit(const rumur::SwitchCase &n) final;
  void visit(const rumur::Ternary &n) final;
  void visit(const rumur::TypeDecl &n) final;
  void visit(const rumur::TypeExprID &n) final;
  void visit(const rumur::Undefine &n) final;
  void visit(const rumur::VarDecl &n) final;
  void visit(const rumur::While &n) final;

  virtual ~XMLPrinter();

 private:
  void add_location(const rumur::Node &n);
  void visit_bexpr(const std::string &tag, const rumur::BinaryExpr &n);
  void visit_uexpr(const std::string &tag, const rumur::UnaryExpr &n);
  void sync_to(const rumur::Node &n);
  void sync_to(const rumur::position &pos
    = rumur::position(nullptr, UINT_MAX, UINT_MAX));
};
