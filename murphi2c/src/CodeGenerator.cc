#include "CodeGenerator.h"
#include <cassert>
#include <cstddef>
#include <string>

std::string CodeGenerator::indentation() const {
  return std::string(indent_level * 2, ' ');
}

void CodeGenerator::indent() { indent_level++; }

void CodeGenerator::dedent() {
  assert(indent_level > 0 && "attempted negative indentation");
  indent_level--;
}

CodeGenerator::~CodeGenerator() {}
