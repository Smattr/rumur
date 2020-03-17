#include <cstddef>
#include <ctype.h>
#include "Stage.h"
#include <string>
#include "ToAscii.h"

ToAscii::ToAscii(Stage &next_): IntermediateStage(next_) { }

void ToAscii::write(const std::string &c) {

  if (pending_space) {
    if (c.size() != 1 || !isspace(c[0]))
      next.write(" ");
    pending_space = false;
  }

  bool in_idle = state == IDLE || state == IDLE_DASH || state == IDLE_SLASH;

  std::string s = c;

  if (in_idle) {
    if      (s == "“") s = "\"";
    else if (s == "≔") s = ":=";
    else if (s == "≥") s = ">=";
    else if (s == "→") s = "->";
    else if (s == "≤") s = "<=";
    else if (s == "≠") s = "!=";
    else if (s == "⇒") s = "==>";
    else if (s == "¬") s = "!";
    else if (s == "∧") s = "&";
    else if (s == "∨") s = "|";
    else if (s == "∀") { s = "forall"; pending_space = true; }
    else if (s == "∃") { s = "exists"; pending_space = true; }
  }

  switch (state) {

    case IDLE:
      next << s;
      if (s == "-") {
        state = IDLE_DASH;
      } else if (s == "/") {
        state = IDLE_SLASH;
      } else if (s == "\"") {
        state = IN_STRING;
      }
      break;

    case IDLE_DASH:
      next << s;
      if (s == "-") {
        state = IN_LINE_COMMENT;
      } else if (s == "/") {
        state = IDLE_SLASH;
      } else if (s == "\"") {
        state = IN_STRING;
      } else {
        state = IDLE;
      }
      break;

    case IDLE_SLASH:
      next << s;
      if (s == "-") {
        state = IDLE_DASH;
      } else if (s == "/") {
        // stay in IDLE_SLASH
      } else if (s == "*") {
        state = IN_MULTILINE_COMMENT;
      } else if (s == "\"") {
        state = IN_STRING;
      } else {
        state = IDLE;
      }
      break;

    case IN_STRING:
      if (s == "\"" || s == "”") {
        next.write("\"");
        state = IDLE;
      } else {
        next.write(s);
        if (s == "\\")
          state = IN_STRING_SLASH;
      }
      break;

    case IN_STRING_SLASH:
      next.write(s);
      state = IN_STRING;
      break;

    case IN_LINE_COMMENT:
      next.write(s);
      if (s == "\n")
        state = IDLE;
      break;

    case IN_MULTILINE_COMMENT:
      next.write(s);
      if (s == "*")
        state = IN_MULTILINE_COMMENT_STAR;
      break;

    case IN_MULTILINE_COMMENT_STAR:
      next.write(s);
      if (s == "*") {
        // stay in IN_MULTILINE_COMMENT_STAR
      } else if (s == "/") {
        state = IDLE;
      }
      break;
  }
}
