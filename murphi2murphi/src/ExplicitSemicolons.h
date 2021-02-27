#pragma once

#include "Stage.h"
#include <cstddef>
#include <queue>
#include <rumur/rumur.h>
#include <vector>

class ExplicitSemicolons : public IntermediateStage {

private:
  // does the next written character need to be a semicolon?
  bool pending_semi = false;

  // buffered tokens we have not yet sent to the next stage
  std::vector<Token> pending;

  // queued updates to .pending_semi
  std::queue<bool> state;

public:
  explicit ExplicitSemicolons(Stage &next_);

  void process(const Token &t) final;

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

  // queue an update of .pending_semi = true
  void set_pending_semi();
};
