#include "lex.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/// Murphi and extended Rumur operators
static const char *const OPERATORS[] = {
    ":=",  "≔", "..", ">=", "≥", ">>", "->", "→",  "<=", "≤", "<<", "!=", "≠",
    "==>", "⇒", "==", "¬",  "∧", "∨",  "&&", "||", "÷",  "−", "∕",  "×",  "=",
    "+",   "-", "*",  "/",  "%", "&",  "|",  "^",  "!",  "?", "<",  ">",  ":",
};

/// does this look like part of a Murphi operator?
///
/// Note that this does not cover everything that is technically a Murphi
/// operator (e.g. '[' and ']'). It only covers the things we want to think of
/// as generic operators.
///
/// \param offset Index within the current token we are looking at
/// \param c Character at this index
/// \return True if this could be part of a known operator
static bool may_be_operator(size_t offset, char c) {
  for (size_t i = 0; i < sizeof(OPERATORS) / sizeof(OPERATORS[0]); ++i) {
    const char *const op = OPERATORS[i];
    if (strlen(op) <= offset)
      continue;
    if (op[offset] == c)
      return true;
  }
  return false;
}

int lex_get_token(lex_t *me, token_t *token) {
  assert(me != NULL);
  assert(token != NULL);

  // Drop temporary string from last call. No need to zero out the backing
  // memory because we always append a '\0' before returning.
  rewind(me->stage);

  // swallow whitespace
  size_t newlines = 0;
  while (!me->done) {
    const int c = getc(me->src);
    if (c == EOF) {
      me->done = true;
      break;
    }
    if (c != ' ' && c != '\f' && c != '\r' && c != '\n') {
      (void)ungetc(c, me->src);
      break;
    }
    if (c == '\n')
      ++newlines;
  }

  if (newlines >= 2) {
    *token = (token_t){.type = TOKEN_BREAK};
    return 0;
  }

  if (me->done) {
    *token = (token_t){.type = TOKEN_EOF};
    return 0;
  }

  const int first = getc(me->src);

  if (first == EOF) {
    *token = (token_t){.type = TOKEN_EOF};
    return 0;
  }

#define ACCRUE(ch)                                                             \
  do {                                                                         \
    if (fputc((ch), me->stage) < 0) {                                          \
      return EIO;                                                              \
    }                                                                          \
  } while (0)

  ACCRUE(first);

#define RET(typ)                                                               \
  do {                                                                         \
    if (fputc('\0', me->stage) < 0) {                                          \
      return EIO;                                                              \
    }                                                                          \
    if (fflush(me->stage) < 0) {                                               \
      return errno;                                                            \
    }                                                                          \
    *token = (token_t){.type = (typ), .text = me->stage_base};                 \
    return 0;                                                                  \
  } while (0)

  switch (first) {
  case '0':
  case '1' ... '9':
    while (true) {

      const int c = getc(me->src);
      if (c == EOF) {
        me->done = true;
        break;
      }
      if (c < '0' || c > '9') {
        ungetc(c, me->src);
        break;
      }
      ACCRUE(c);
    }
    RET(TOKEN_NUMBER);

  case 'a' ... 'z':
  case 'A' ... 'Z':
  case '_':
    while (true) {
      const int c = getc(me->src);
      if (c == EOF) {
        me->done = true;
        break;
      }
      if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'z') &&
          !(c >= 'A' && c <= 'Z') && c != '_') {
        ungetc(c, me->src);
        break;
      }
      ACCRUE(c);
    }
    RET(TOKEN_ID);

  case ';':
    RET(TOKEN_SEMI);

  case 0xe2: { // first byte of "“" and "”"
    const int second = getc(me->src);
    if (second == EOF) {
      me->done = true;
      break;
    }
    if (second != 0x80) {
      ungetc(second, me->src);
      break;
    }
    ACCRUE(second);
    const int third = getc(me->src);
    if (third == EOF) {
      me->done = true;
      break;
    }
    if (third != 0x9c && third != 0x9d) {
      ungetc(third, me->src);
      break;
    }
    ACCRUE(third);
    if (third == 0x9d) // we have "”"
      break;
  }
  // fall through
  case '"':
    for (bool escaping = false;;) {
      const int c = getc(me->src);
      if (c == EOF) {
        me->done = true;
        break;
      }
      ACCRUE(c);
      if (fflush(me->stage) < 0)
        return errno;
      if (!escaping) {
        if (me->stage_base[0] == '"') {
          if (me->stage_base[me->stage_size - 1] == '"')
            break;
        }
        if (me->stage_size >= strlen("“") + strlen("”")) {
          if (strncmp(me->stage_base, "“", strlen("“")) == 0) {
            if (strcmp(&me->stage_base[me->stage_size - strlen("”")], "”") == 0)
              break;
          }
        }
      }
      if (c == '\\') {
        escaping = !escaping;
      } else {
        escaping = false;
      }
    }
    RET(TOKEN_STRING);

  case '(':
  case '[':
    RET(TOKEN_OPEN_PAREN);

  case ')':
  case ']':
    RET(TOKEN_CLOSE_PAREN);

  case '.':
    RET(TOKEN_DOT);

  case ',':
    RET(TOKEN_COMMA);

  case '{':
    RET(TOKEN_OPEN_BRACE);

  default:
    if (may_be_operator(0, first)) {
      for (size_t i = 1;; ++i) {
        const int c = getc(me->src);
        if (c == EOF) {
          me->done = true;
          break;
        }
        if (first == '-' && i == 1 && c == '-') {
          // this is a comment
          ACCRUE(c);
          bool is_raw = false;
          while (true) {
            if (fflush(me->stage) < 0) {
              return EIO;
            }
            const int n = getc(me->src);
            if (n == EOF) {
              me->done = true;
              break;
            }
            if (n == '\n') {
              if (!is_raw) {
                static const char OFF[] = "-- murphi-format: off";
                if (strcmp(me->stage_base, OFF) == 0) {
                  is_raw = true;
                } else {
                  ungetc(n, me->src);
                  break;
                }
              } else {
                static const char ON[] = "-- murphi-format: on";
                if (strcmp(me->stage_base, ON) == 0) {
                  ungetc(n, me->src);
                  break;
                }
              }
            }
            ACCRUE(n);
          }
          if (is_raw)
            RET(TOKEN_RAW);
          if (newlines > 0)
            RET(TOKEN_NL_COMMENT);
          RET(TOKEN_LINE_COMMENT);
        }
        if (first == '/' && i == 1 && c == '*') {
          // this is a multiline comment
          ACCRUE(c);
          for (bool saw_star = false;;) {
            const int n = getc(me->src);
            if (n == EOF) {
              me->done = true;
              break;
            }
            ACCRUE(n);
            if (saw_star && n == '/')
              break;
            saw_star = n == '*';
          }
          RET(TOKEN_MULTILINE_COMMENT);
        }
        if (!may_be_operator(i, c)) {
          ungetc(c, me->src);
          break;
        }
        ACCRUE(c);
      }
      RET(TOKEN_OPERATOR);
      return 0;
    }
  }

  RET(TOKEN_UNKNOWN);
#undef RET
#undef ACCRUE
}
