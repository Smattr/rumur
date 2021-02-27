#include "escape.h"
#include <cstddef>
#include <cstdio>
#include <ctype.h>
#include <string>

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
