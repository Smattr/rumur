#pragma once

#include <cstddef>
#include <rumur/rumur.h>
#include <string>

class Stage;

// a message sent to a stage
struct Token {
  enum {
    CHAR, // write the given character to the output
    SUBJ, // update to your next internal pending state
  } type;

  // only one of these is ever active, but we avoid using a union so we still
  // get default ctors etc
  std::string character;
  const Stage *subject = nullptr;

  explicit Token(const std::string &c) : type(CHAR), character(c) {}
  explicit Token(const Stage *s) : type(SUBJ), subject(s) {}
};

class Stage : public rumur::ConstBaseTraversal {

protected:
  Stage *top = nullptr;

public:
  // pass one or more characters to process()
  Stage &operator<<(const std::string &s);

  // make this stage aware it is part of the given pipeline
  void attach(Stage &top_);

  // process a token, either a character to write to the output stream or a
  // notification to shift state
  virtual void process(const Token &t) = 0;

  // write characters from the input file into the output file, up to the given
  // position in the input
  virtual void sync_to(const rumur::Node &n) = 0;
  virtual void sync_to(const rumur::position &pos) = 0;

  // seek over characters in the input file up to the given position in the
  // input
  virtual void skip_to(const rumur::Node &n) = 0;
  virtual void skip_to(const rumur::position &pos) = 0;

  // perform any pending actions, assuming that processing is done
  virtual void finalise(){};

  virtual ~Stage() = default;
};

class IntermediateStage : public Stage {

protected:
  Stage &next;

public:
  explicit IntermediateStage(Stage &next_);

  void visit_add(const rumur::Add &n) override;
  void visit_aliasdecl(const rumur::AliasDecl &n) override;
  void visit_aliasrule(const rumur::AliasRule &n) override;
  void visit_aliasstmt(const rumur::AliasStmt &n) override;
  void visit_and(const rumur::And &n) override;
  void visit_array(const rumur::Array &n) override;
  void visit_assignment(const rumur::Assignment &n) override;
  void visit_band(const rumur::Band &n) override;
  void visit_bnot(const rumur::Bnot &n) override;
  void visit_bor(const rumur::Bor &n) override;
  void visit_clear(const rumur::Clear &n) override;
  void visit_constdecl(const rumur::ConstDecl &n) override;
  void visit_div(const rumur::Div &n) override;
  void visit_element(const rumur::Element &n) override;
  void visit_enum(const rumur::Enum &n) override;
  void visit_eq(const rumur::Eq &n) override;
  void visit_errorstmt(const rumur::ErrorStmt &n) override;
  void visit_exists(const rumur::Exists &n) override;
  void visit_exprid(const rumur::ExprID &n) override;
  void visit_field(const rumur::Field &n) override;
  void visit_for(const rumur::For &n) override;
  void visit_forall(const rumur::Forall &n) override;
  void visit_function(const rumur::Function &n) override;
  void visit_functioncall(const rumur::FunctionCall &n) override;
  void visit_geq(const rumur::Geq &n) override;
  void visit_gt(const rumur::Gt &n) override;
  void visit_if(const rumur::If &n) override;
  void visit_ifclause(const rumur::IfClause &n) override;
  void visit_implication(const rumur::Implication &n) override;
  void visit_isundefined(const rumur::IsUndefined &n) override;
  void visit_leq(const rumur::Leq &n) override;
  void visit_lsh(const rumur::Lsh &n) override;
  void visit_lt(const rumur::Lt &n) override;
  void visit_mod(const rumur::Mod &n) override;
  void visit_model(const rumur::Model &n) override;
  void visit_mul(const rumur::Mul &n) override;
  void visit_negative(const rumur::Negative &n) override;
  void visit_neq(const rumur::Neq &n) override;
  void visit_not(const rumur::Not &n) override;
  void visit_number(const rumur::Number &n) override;
  void visit_or(const rumur::Or &n) override;
  void visit_procedurecall(const rumur::ProcedureCall &n) override;
  void visit_property(const rumur::Property &n) override;
  void visit_propertyrule(const rumur::PropertyRule &n) override;
  void visit_propertystmt(const rumur::PropertyStmt &n) override;
  void visit_put(const rumur::Put &n) override;
  void visit_quantifier(const rumur::Quantifier &n) override;
  void visit_range(const rumur::Range &n) override;
  void visit_record(const rumur::Record &n) override;
  void visit_return(const rumur::Return &n) override;
  void visit_rsh(const rumur::Rsh &n) override;
  void visit_ruleset(const rumur::Ruleset &n) override;
  void visit_scalarset(const rumur::Scalarset &n) override;
  void visit_simplerule(const rumur::SimpleRule &n) override;
  void visit_startstate(const rumur::StartState &n) override;
  void visit_sub(const rumur::Sub &n) override;
  void visit_switch(const rumur::Switch &n) override;
  void visit_switchcase(const rumur::SwitchCase &n) override;
  void visit_ternary(const rumur::Ternary &n) override;
  void visit_typedecl(const rumur::TypeDecl &n) override;
  void visit_typeexprid(const rumur::TypeExprID &n) override;
  void visit_undefine(const rumur::Undefine &n) override;
  void visit_vardecl(const rumur::VarDecl &n) override;
  void visit_while(const rumur::While &n) override;
  void visit_xor(const rumur::Xor &n) override;

  void process(const Token &t) override;

  void sync_to(const rumur::Node &n) override;
  void sync_to(const rumur::position &pos) override;

  void skip_to(const rumur::Node &n) override;
  void skip_to(const rumur::position &pos) override;

  virtual ~IntermediateStage() = 0;
};
