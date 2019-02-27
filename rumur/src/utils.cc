#include <cstdio>
#include <ctype.h>
#include <rumur/rumur.h>
#include <string>
#include "utils.h"

static std::string octal(char c) {
  char buffer[sizeof("\\000")];
  snprintf(buffer, sizeof(buffer), "\\%03o", c);
  return buffer;
}

std::string escape(const std::string &s) {
  std::string out;
  for (const char &c : s) {
    if (iscntrl(c) || c == '\\' || c == '\"') {
      out += "\\" + octal(c);
    } else {
      out += c;
    }
  }
  return out;
}

std::string to_C_string(const rumur::Expr &expr) {
  return "\"" + escape(expr.to_string()) + "\"";
}
