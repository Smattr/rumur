#include "format.h"
#include "compiler.h"
#include "debug.h"
#include "lex.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

struct state;

/// a modifier action, that can tweak the state based on the next token
///
/// Modifiers can freely mutate the reformatter given to them. They are usually
/// entered with themselves registered (`state->mod == ourselves`) and are
/// expected to deregister themselves (`state->mod = NULL`) on completion.
///
/// \param state The state of the in-progress reformatting operation
/// \param token The next token, about to be printed
/// \return 0 on success or an errno on failure
typedef int (*mod_t)(struct state *state, token_t token);

/// state of the reformatter
typedef struct state {
  FILE *dst;               ///< destination stream
  size_t indentation;      ///< current levels of indent
  size_t soft_indentation; ///< additionally indent levels that may be reset
                           ///< implicitly

  /// Are we currently within a ternary expression? This is only tracked
  /// inaccurately. We use this as a heuristic to tune the spacing around ':'.
  bool in_ternary;

  /// How many '(…)' are we within? This is tracked for the purposes of
  /// disambiguating 'var'.
  size_t paren_nesting;

  /// the token we last saw
  token_type_t previous;

  /// if previous == TOKEN_ID, was it a keyword?
  bool previous_was_keyword;

  /// Optional modifier installed by the last token. This effectively gives the
  /// reformatter a 1-lookahead ability, wherein the actions for a token can
  /// also take the next token into account in their logic.
  mod_t mod;

  /// have we output anything to `dst` yet?
  bool started;
} state_t;

/// are two strings equal?
static bool streq(const char *a, const char *b) {
  return strcasecmp(a, b) == 0;
}

/// handle an implicit newline following the previous token
static int pend_newline(state_t *st, token_t token) {
  assert(st != NULL);

  int rc = 0;

  // no need for a newline if we just incurred one
  if (token.type == TOKEN_BREAK)
    goto done;

  if (fputc('\n', st->dst) < 0) {
    rc = EIO;
    goto done;
  }
  if (token.type == TOKEN_EOF)
    goto done;
  size_t indentation = st->indentation;
  if (token.type != TOKEN_ID || !streq(token.text, "begin"))
    indentation += st->soft_indentation;
  for (size_t i = 0; i < indentation; ++i) {
    if (fputs(tab, st->dst) < 0) {
      rc = EIO;
      goto done;
    }
  }

done:
  // de-register ourselves
  st->mod = NULL;

  return rc;
}

/// handle an implicit space following the previous token
static int pend_space(state_t *st, token_t token) {
  assert(st != NULL);

  int rc = 0;

  // avoid trailing spaces in output
  if (token.type == TOKEN_EOF)
    goto done;

  // no need for a space if we are at the start of a line
  if (token.type == TOKEN_BREAK)
    goto done;

  // suppress pending spacing for closing parens
  if (token.type == TOKEN_CLOSE_PAREN)
    goto done;

  // suppress pending spacing for dot
  if (token.type == TOKEN_DOT)
    goto done;

  // conditionally suppress pending spacing for opening parens
  if (token.type == TOKEN_OPEN_PAREN) {
    if ((st->previous == TOKEN_ID && !st->previous_was_keyword) ||
        st->previous == TOKEN_OPEN_PAREN || st->previous == TOKEN_CLOSE_PAREN)
      goto done;
  }

  // suppress spacing before ':' if it looks like we are not in a ternary
  if (!st->in_ternary) {
    if (token.type == TOKEN_OPERATOR && streq(token.text, ":"))
      goto done;
  }

  if (fputc(' ', st->dst) < 0) {
    rc = EIO;
    goto done;
  }

done:
  // de-register ourselves
  st->mod = NULL;

  return rc;
}

/// modifier action for just having seen `==>`
static int arrow_lookahead(state_t *st, token_t token) {
  assert(st != NULL);

  int rc = 0;

  // if this rule is being explicitly begun, all we need is a space
  if (token.type == TOKEN_ID && streq(token.text, "begin")) {
    rc = pend_space(st, token);
    goto done;
  }

  // otherwise infer a newline and indent
  ++st->indentation;
  rc = pend_newline(st, token);

done:
  // de-register ourselves
  st->mod = NULL;

  return rc;
}

/// modifier action for just having seen `startstate`
static int startstate_lookahead(state_t *st, token_t token) {
  assert(st != NULL);

  int rc = 0;

  if (token.type != TOKEN_ID || !streq(token.text, "begin"))
    ++st->indentation;

  // if this does not look like a string, we probably have the implicit
  // beginning of the startstate body
  if (token.type != TOKEN_STRING) {
    rc = pend_newline(st, token);
    goto done;
  }

done:
  // de-register ourselves
  st->mod = NULL;

  return rc;
}

/// is this a Murphi/Rumur keyword?
static bool is_keyword(const char *text) {
  assert(text != NULL);

  if (streq(text, "alias"))
    return true;
  if (streq(text, "array"))
    return true;
  if (streq(text, "assert"))
    return true;
  if (streq(text, "assume"))
    return true;
  if (streq(text, "begin"))
    return true;
  if (streq(text, "boolean"))
    return true;
  if (streq(text, "by"))
    return true;
  if (streq(text, "case"))
    return true;
  if (streq(text, "clear"))
    return true;
  if (streq(text, "const"))
    return true;
  if (streq(text, "cover"))
    return true;
  if (streq(text, "do"))
    return true;
  if (streq(text, "else"))
    return true;
  if (streq(text, "elsif"))
    return true;
  if (streq(text, "end"))
    return true;
  if (streq(text, "endalias"))
    return true;
  if (streq(text, "endexists"))
    return true;
  if (streq(text, "endfor"))
    return true;
  if (streq(text, "endforall"))
    return true;
  if (streq(text, "endfunction"))
    return true;
  if (streq(text, "endif"))
    return true;
  if (streq(text, "endprocedure"))
    return true;
  if (streq(text, "endrecord"))
    return true;
  if (streq(text, "endrule"))
    return true;
  if (streq(text, "endruleset"))
    return true;
  if (streq(text, "endstartstate"))
    return true;
  if (streq(text, "endswitch"))
    return true;
  if (streq(text, "endwhile"))
    return true;
  if (streq(text, "enum"))
    return true;
  if (streq(text, "error"))
    return true;
  if (streq(text, "exists"))
    return true;
  if (streq(text, "for"))
    return true;
  if (streq(text, "forall"))
    return true;
  if (streq(text, "function"))
    return true;
  if (streq(text, "if"))
    return true;
  if (streq(text, "invariant"))
    return true;
  if (streq(text, "isundefined"))
    return true;
  if (streq(text, "liveness"))
    return true;
  if (streq(text, "of"))
    return true;
  if (streq(text, "procedure"))
    return true;
  if (streq(text, "put"))
    return true;
  if (streq(text, "real"))
    return true;
  if (streq(text, "record"))
    return true;
  if (streq(text, "return"))
    return true;
  if (streq(text, "rule"))
    return true;
  if (streq(text, "ruleset"))
    return true;
  if (streq(text, "scalarset"))
    return true;
  if (streq(text, "startstate"))
    return true;
  if (streq(text, "switch"))
    return true;
  if (streq(text, "then"))
    return true;
  if (streq(text, "to"))
    return true;
  if (streq(text, "type"))
    return true;
  if (streq(text, "undefine"))
    return true;
  if (streq(text, "union"))
    return true;
  if (streq(text, "var"))
    return true;
  if (streq(text, "while"))
    return true;

  return false;
}

/// is the given text a unary operator?
static bool is_unary(state_t st, const char *text) {
  assert(text != NULL);

  if (streq(text, "!"))
    return true;
  if (streq(text, "~"))
    return true;

  if (streq(text, "+") || streq(text, "-")) {
    if (st.previous_was_keyword)
      return true;
    if (st.previous == TOKEN_ID)
      return false;
    if (st.previous == TOKEN_NUMBER)
      return false;
    if (st.previous == TOKEN_CLOSE_PAREN)
      return false;
    return true;
  }

  return false;
}

/// does the given text start a paragraph block?
static bool is_block_starter(state_t st, const char *text) {

  if (text == NULL)
    return false;

  if (streq(text, "aliasrule"))
    return true;
  if (streq(text, "const"))
    return true;
  if (streq(text, "invariant"))
    return true;
  if (streq(text, "function"))
    return true;
  if (streq(text, "procedure"))
    return true;
  if (streq(text, "rule"))
    return true;
  if (streq(text, "ruleset"))
    return true;
  if (streq(text, "startstate"))
    return true;
  if (streq(text, "type"))
    return true;
  if (streq(text, "var") && st.paren_nesting == 0)
    return true;
  return false;
}

/// does this start an indented block?
static bool is_indenter(const char *text) {
  if (text == NULL)
    return false;
  if (streq(text, "begin"))
    return true;
  if (streq(text, "do"))
    return true;
  if (streq(text, "else"))
    return true;
  if (streq(text, "record"))
    return true;
  if (streq(text, "then"))
    return true;
  return false;
}

/// does this start a soft-indented block?
static bool is_soft_indenter(const char *text) {
  if (text == NULL)
    return false;
  if (streq(text, "const"))
    return true;
  if (streq(text, "type"))
    return true;
  if (streq(text, "var"))
    return true;
  return false;
}

/// does this imply a preceding dedent?
static bool is_dedenter(const char *text) {
  if (text == NULL)
    return false;
  if (streq(text, "else"))
    return true;
  if (streq(text, "elsif"))
    return true;
  if (streq(text, "end"))
    return true;
  if (streq(text, "endalias"))
    return true;
  if (streq(text, "endexists"))
    return true;
  if (streq(text, "endfor"))
    return true;
  if (streq(text, "endfunction"))
    return true;
  if (streq(text, "endif"))
    return true;
  if (streq(text, "endprocedure"))
    return true;
  if (streq(text, "endrecord"))
    return true;
  if (streq(text, "endrule"))
    return true;
  if (streq(text, "endruleset"))
    return true;
  if (streq(text, "endstartstate"))
    return true;
  if (streq(text, "endswitch"))
    return true;
  if (streq(text, "endwhile"))
    return true;
  if (streq(text, "}"))
    return true;
  return false;
}

/// is this semantically the `==>` operator?
static bool is_arrow(const char *text) {
  if (text == NULL)
    return false;
  if (streq(text, "==>"))
    return true;
  if (streq(text, "⇒"))
    return true;
  return false;
}

int format(FILE *dst, FILE *src) {

  state_t st = {.dst = dst};
  lex_t lex = {0};
  int rc = 0;

  if ((rc = lex_new(&lex, src)))
    goto done;

  while (true) {

    token_t tok;
    if ((rc = lex_get_token(&lex, &tok)))
      goto done;

    DEBUG("saw token %d with text %s", (int)tok.type,
          tok.text == NULL ? "<null>" : tok.text);

    // allow ; to suppress space and sneak in before newline
    if (tok.type == TOKEN_SEMI) {
      if (fputc(';', dst) < 0) {
        rc = EIO;
        goto done;
      }
      st.mod = pend_newline;
      st.previous = tok.type;
      st.in_ternary = false;
      st.started = true;
      continue;
    }

    // allow ',' to suppress space
    if (tok.type == TOKEN_COMMA && st.mod == pend_space)
      st.mod = NULL;

    if (is_block_starter(st, tok.text)) {
      if (st.started && st.previous != TOKEN_BREAK) {
        st.mod = pend_newline;
      }
      st.soft_indentation = 0;
    }

    if (is_dedenter(tok.text) && st.indentation > 0)
      --st.indentation;
    if (is_dedenter(tok.text) && !streq(tok.text, "}") && st.mod == pend_space)
      st.mod = pend_newline;

    const bool newlined = st.mod == pend_newline || st.previous == TOKEN_BREAK;
    if (st.mod != NULL)
      st.mod(&st, tok);

    if (tok.type == TOKEN_EOF)
      break;

    switch (tok.type) {

    case TOKEN_ID:
    case TOKEN_NUMBER:
    case TOKEN_STRING:
    case TOKEN_UNKNOWN:
    case TOKEN_OPERATOR:
    case TOKEN_CLOSE_PAREN:
    case TOKEN_COMMA:
      if (fputs(tok.text, dst) < 0) {
        rc = EIO;
        goto done;
      }
      if (is_indenter(tok.text)) {
        st.mod = pend_newline;
        if (streq(tok.text, "begin"))
          st.soft_indentation = 0;
        ++st.indentation;
      } else if (streq(tok.text, "startstate")) {
        st.mod = startstate_lookahead;
      } else if (st.paren_nesting == 0 && is_soft_indenter(tok.text)) {
        st.mod = pend_newline;
        ++st.soft_indentation;
      } else if (is_arrow(tok.text)) {
        st.mod = arrow_lookahead;
      } else if ((is_dedenter(tok.text) && !streq(tok.text, "elsif")) ||
                 streq(tok.text, ";")) {
        st.mod = pend_newline;
      } else if (!is_unary(st, tok.text)) {
        st.mod = pend_space;
      }
      break;

    case TOKEN_OPEN_PAREN:
    case TOKEN_DOT:
      if (fputs(tok.text, dst) < 0) {
        rc = EIO;
        goto done;
      }
      break;

    case TOKEN_OPEN_BRACE:
      if (fputc('{', dst) < 0) {
        rc = EIO;
        goto done;
      }
      ++st.indentation;
      st.mod = pend_space;
      break;

    case TOKEN_NL_COMMENT:
      if (!newlined) {
        if (fputc('\n', dst) < 0) {
          rc = EIO;
          goto done;
        }
        for (size_t i = 0; i < st.indentation + st.soft_indentation; ++i) {
          if (fputs(tab, dst) < 0) {
            rc = EIO;
            goto done;
          }
        }
      }
      // fall through
    case TOKEN_LINE_COMMENT:
      if (fputs(tok.text, dst) < 0) {
        rc = EIO;
        goto done;
      }
      st.mod = pend_newline;
      break;

    case TOKEN_MULTILINE_COMMENT:
      for (size_t i = 0; tok.text[i] != '\0';) {
        if (fputc(tok.text[i], dst) < 0) {
          rc = EIO;
          goto done;
        }
        ++i;
        if (tok.text[i - 1] == '\n') {
          // override indentation with our own
          while (tok.text[i] == ' ' || tok.text[i] == '\t')
            ++i;
          for (size_t j = 0; j < st.indentation + st.soft_indentation; ++j) {
            if (fputs(tab, dst) < 0) {
              rc = EIO;
              goto done;
            }
          }
          if (tok.text[i] == '*') {
            if (fputc(' ', dst) < 0) {
              rc = EIO;
              goto done;
            }
          }
        }
      }
      st.mod = pend_newline;
      break;

    case TOKEN_BREAK:
      if (fputc('\n', dst) < 0) {
        rc = EIO;
        goto done;
      }
      st.mod = pend_newline;
      break;

    case TOKEN_SEMI:
    case TOKEN_EOF:
      UNREACHABLE();
    }

    if (tok.type == TOKEN_OPERATOR && streq(tok.text, "?"))
      st.in_ternary = true;
    if (tok.type == TOKEN_OPEN_PAREN && streq(tok.text, "("))
      ++st.paren_nesting;
    if (tok.type == TOKEN_CLOSE_PAREN && streq(tok.text, ")")) {
      if (st.paren_nesting > 0)
        --st.paren_nesting;
    }
    if (tok.type == TOKEN_ID && streq(tok.text, "switch"))
      ++st.indentation;

    st.previous = tok.type;
    st.previous_was_keyword =
        tok.type == TOKEN_ID ? is_keyword(tok.text) : false;
    st.started = true;
  }

done:
  lex_free(&lex);

  return rc;
}
