#pragma once

#include <cstddef>
#include "CodeGenerator.h"
#include <iostream>
#include <rumur/rumur.h>
#include <string>

// generator for C-like code
class CLikeGenerator : public CodeGenerator, public rumur::ConstBaseTraversal {

 protected:
  std::ostream &out;
  bool pack;

 public:
  CLikeGenerator(std::ostream &out_, bool pack_): out(out_), pack(pack_) { }

  void visit_add(const rumur::Add &n) final;
  void visit_and(const rumur::And &n) final;
  void visit_div(const rumur::Div &n) final;
  void visit_element(const rumur::Element &n) final;
  void visit_eq(const rumur::Eq &n) final;
  void visit_exists(const rumur::Exists &n) final;
  void visit_exprid(const rumur::ExprID &n) final;
  void visit_field(const rumur::Field &n) final;
  void visit_implication(const rumur::Implication &n) final;
  void visit_isundefined(const rumur::IsUndefined&) final;
  void visit_forall(const rumur::Forall &n) final;
  void visit_functioncall(const rumur::FunctionCall &n) final;
  void visit_geq(const rumur::Geq &n) final;
  void visit_gt(const rumur::Gt &n) final;
  void visit_leq(const rumur::Leq &n) final;
  void visit_lt(const rumur::Lt &n) final;
  void visit_mod(const rumur::Mod &n) final;
  void visit_mul(const rumur::Mul &n) final;
  void visit_negative(const rumur::Negative &n) final;
  void visit_neq(const rumur::Neq &n) final;
  void visit_not(const rumur::Not &n) final;
  void visit_number(const rumur::Number &n) final;
  void visit_or(const rumur::Or &n) final;
  void visit_procedurecall(const rumur::ProcedureCall &n) final;
  void visit_property(const rumur::Property&) final;
  void visit_sub(const rumur::Sub &n) final;
  void visit_ternary(const rumur::Ternary &n) final;
  void visit_undefine(const rumur::Undefine &n) final;

  // helpers to make output more natural
  CLikeGenerator &operator<<(const std::string &s);
  CLikeGenerator &operator<<(const rumur::Node &n);

  // make this class abstract
  virtual ~CLikeGenerator() = 0;
};
