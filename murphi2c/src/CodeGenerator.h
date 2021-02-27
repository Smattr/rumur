#pragma once

#include <cstddef>
#include <string>

// some common supporting code used by code generation functions
class CodeGenerator {

private:
  size_t indent_level = 0; // current indentation level

protected:
  // get a white space string for the current indentation level
  std::string indentation() const;

  // increase the current indentation level
  void indent();

  // decrease the current indentation level
  void dedent();

public:
  // make this class abstract
  virtual ~CodeGenerator() = 0;
};
