#include <cstddef>
#include "CLikeGenerator.h"
#include <iostream>
#include <rumur/rumur.h>
#include <string>

using namespace rumur;

CLikeGenerator::CLikeGenerator(std::ostream &out_): out(out_) { }

CLikeGenerator &CLikeGenerator::operator<<(const std::string &s) {
  out << s;
  return *this;
}

CLikeGenerator &CLikeGenerator::operator<<(const Node &n) {
  dispatch(n);
  return *this;
}

CLikeGenerator::~CLikeGenerator() { }
