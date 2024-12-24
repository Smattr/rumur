/// \file
/// \brief Support for robust lexing of Murphi
///
/// This lexer exists separately from ../../librumur/src/lexer.l because the
/// reformatter wants to handle potentially malformed Murphi. This lexer never
/// gives up; any and all text is assumed to be valid-ish Murphi.

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/// state of the lexer
typedef struct {
  FILE *src; ///< source file being lexed
  bool done; ///< have we seen EOF?

  /// buffer for constructing temporary strings
  FILE *stage;
  char *stage_base;
  size_t stage_size;
} lex_t;

/// the kind of a token in the input stream
typedef enum {
  TOKEN_UNKNOWN = 0,
  TOKEN_EOF, // end-of-file
  TOKEN_ID,
  TOKEN_NUMBER,
  TOKEN_STRING,
  TOKEN_OPERATOR,
  TOKEN_OPEN_PAREN,
  TOKEN_CLOSE_PAREN,
  TOKEN_OPEN_BRACE,
  TOKEN_DOT,
  TOKEN_COMMA,
  TOKEN_SEMI,
  TOKEN_LINE_COMMENT,
  TOKEN_NL_COMMENT, // comment with a preceding hard \n
  TOKEN_MULTILINE_COMMENT,
  TOKEN_BREAK, // double \n
} token_type_t;

/// a token encountered in the input stream
typedef struct {
  token_type_t type;
  const char *text; ///< only non-null for some token types
} token_t;

/// create a new lexer
///
/// \param me [out] Created lexer on success
/// \param src Source stream to lex
/// \return 0 on success or an errno on failure
int lex_new(lex_t *me, FILE *src);

/// retrieve the next token in the input stream
///
/// \param me Lexer state
/// \param token [out] Next token on success
/// \return 0 on success or an errno on failure
int lex_get_token(lex_t *me, token_t *token);

/// cleanup a lexer
///
/// \param me Lexer to destroy
void lex_free(lex_t *me);
