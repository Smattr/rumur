#pragma once

#include "Stage.h"
#include <cstddef>
#include <rumur/rumur.h>
#include <string>

// a stage for turning switch statements into if statements
class SwitchToIf : public IntermediateStage {

private:
  // have we figured out what character(s) are used for indentation?
  bool learned_indentation = false;

  // was the last relevant character we saw a newline?
  bool last_newline = false;

  // the character(s) we believe to represent one level of indentation
  std::string indentation;

public:
  explicit SwitchToIf(Stage &next_);

  void process(const Token &t) final;

  void visit_switch(const rumur::Switch &n) final;

  virtual ~SwitchToIf() = default;
};
