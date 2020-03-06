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

 public:
  explicit CLikeGenerator(std::ostream &out_);

  // helpers to make output more natural
  CLikeGenerator &operator<<(const std::string &s);
  CLikeGenerator &operator<<(const rumur::Node &n);

  // make this class abstract
  virtual ~CLikeGenerator() = 0;
};
