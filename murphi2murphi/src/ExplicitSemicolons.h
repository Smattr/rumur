#pragma once

#include <cstddef>
#include <rumur/rumur.h>
#include <sstream>
#include "Stage.h"
#include <string>

class ExplicitSemicolons : public IntermediateStage {

 private:
  // does the next written character need to be a semicolon?
  bool pending_semi = false;

  // buffered characters we have not yet sent to the next stage
  std::ostringstream pending;

 public:
  explicit ExplicitSemicolons(Stage &next_);

  void write(const std::string &c) final;

  // override visitors for all nodes that can have an omitted semicolon
  void visit_aliasrule(const rumur::AliasRule &n) final;
  void visit_constdecl(const rumur::ConstDecl &n) final;
  void visit_function(const rumur::Function &n) final;
  void visit_propertyrule(const rumur::PropertyRule &n) final;
  void visit_ruleset(const rumur::Ruleset &n) final;
  void visit_simplerule(const rumur::SimpleRule &n) final;
  void visit_startstate(const rumur::StartState &n) final;
  void visit_typedecl(const rumur::TypeDecl &n) final;
  void visit_vardecl(const rumur::VarDecl &n) final;

  void finalise() final;

  virtual ~ExplicitSemicolons() = default;

 private:
  void flush();
};
