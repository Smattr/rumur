#pragma once

#ifndef yyFlexLexerOnce
    #include <FlexLexer.h>
#endif

#include "parser.yy.hh"

namespace rumur {

class scanner : public yyFlexLexer {

  public:

    // Inherit yyFlexLexer's constructor
    using yyFlexLexer::yyFlexLexer;

    // Force a new available type signature for yylex
    virtual int yylex(parser::semantic_type *const lval,
        parser::location_type *loc);

};

}
