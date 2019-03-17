#pragma once

#include <cstddef>

#ifndef yyFlexLexerOnce
  #include <FlexLexer.h>
#endif

#include "parser.yy.hh"

namespace rumur {

class scanner : public yyFlexLexer {

 public:

  // Delegate to yyFlexLexer's constructor
	scanner(FLEX_STD istream* arg_yyin = 0, FLEX_STD ostream* arg_yyout = 0):
    yyFlexLexer(arg_yyin, arg_yyout) { }

/* XXX: Clang's -Woverloaded-virtual decides that the following declaration is
 * possibly a mistake. However, we are deliberately overloading this method with
 * a different type signature. The cleanest way I've found around it is to
 * toggle off the warning here.
 */
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif
  // Force a new available type signature for yylex
  virtual int yylex(parser::semantic_type *const lval,
    parser::location_type *loc);
#ifdef __clang__
  #pragma clang diagnostic pop
#endif

};

}
