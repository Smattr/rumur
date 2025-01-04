#include "ToAscii.h"
#include "Stage.h"
#include <cstddef>
#include <ctype.h>
#include <string>

ToAscii::ToAscii(Stage &next_) : IntermediateStage(next_) {}

void ToAscii::process(const Token &t) {

  // ignore any shift messages
  if (t.type == Token::SUBJ) {
    next.process(t);
    return;
  }

  std::string s = t.character;

  if (pending_space) {
    if (s.size() != 1 || !isspace(s[0]))
      next << " ";
    pending_space = false;
  }

  bool in_idle = state == IDLE || state == IDLE_DASH || state == IDLE_SLASH;

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
    else if (s == "÷") s = "/";
    else if (s == "−") s = "-";
    else if (s == "∕") s = "/";
    else if (s == "×") s = "*";
  }

  switch (state) {

  case IDLE:
    next << s;
    if (t.character == "-") {
      state = IDLE_DASH;
    } else if (t.character == "/") {
      state = IDLE_SLASH;
    } else if (s == "\"") {
      state = IN_STRING;
    }
    break;

  case IDLE_DASH:
    next << s;
    if (t.character == "-") {
      state = IN_LINE_COMMENT;
    } else if (t.character == "/") {
      state = IDLE_SLASH;
    } else if (s == "\"") {
      state = IN_STRING;
    } else {
      state = IDLE;
    }
    break;

  case IDLE_SLASH:
    next << s;
    if (t.character == "-") {
      state = IDLE_DASH;
    } else if (t.character == "/") {
      // stay in IDLE_SLASH
    } else if (t.character == "*") {
      state = IN_MULTILINE_COMMENT;
    } else if (s == "\"") {
      state = IN_STRING;
    } else {
      state = IDLE;
    }
    break;

  case IN_STRING:
    if (s == "\"" || s == "”") {
      next << "\"";
      state = IDLE;
    } else {
      next << s;
      if (s == "\\")
        state = IN_STRING_SLASH;
    }
    break;

  case IN_STRING_SLASH:
    next << s;
    state = IN_STRING;
    break;

  case IN_LINE_COMMENT:
    next << s;
    if (s == "\n")
      state = IDLE;
    break;

  case IN_MULTILINE_COMMENT:
    next << s;
    if (s == "*")
      state = IN_MULTILINE_COMMENT_STAR;
    break;

  case IN_MULTILINE_COMMENT_STAR:
    next << s;
    if (s == "*") {
      // stay in IN_MULTILINE_COMMENT_STAR
    } else if (s == "/") {
      state = IDLE;
    }
    break;
  }
}
