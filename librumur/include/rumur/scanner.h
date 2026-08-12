#pragma once

#include <cstddef>
#include <iostream>

#ifndef yyFlexLexerOnce
#include <FlexLexer.h>
#endif

#include "parser.yy.hh"

#ifndef RUMUR_API_WITH_RTTI
#define RUMUR_API_WITH_RTTI __attribute__((visibility("default")))
#endif

namespace rumur {

class RUMUR_API_WITH_RTTI scanner : public yyFlexLexer {

public:
  // Delegate to yyFlexLexer's constructor
  scanner(std::istream *arg_yyin = 0, std::ostream *arg_yyout = 0)
      : yyFlexLexer(arg_yyin, arg_yyout) {}

  // Force a new available type signature for yylex
  virtual int yylex(parser::semantic_type *const lval,
                    parser::location_type *loc, int &start_token);

  // Hide our parent’s yylex. Basically this tells the compiler, “yes, it is
  // intentional we are overriding `yylex` with a different type signature, do
  // not give us -Woverloaded-virtual warnings”.
private:
  using yyFlexLexer::yylex;
};

} // namespace rumur
