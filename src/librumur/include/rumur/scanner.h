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

/* XXX: Clang's -Woverloaded-virtual decides that the following declaration is
 * possibly a mistake. However, we are deliberately overloading this method with
 * a different type signature. The cleanest way I've found around it is to
 * toggle off the warning here.
 */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
    // Force a new available type signature for yylex
    virtual int yylex(parser::semantic_type *const lval,
        parser::location_type *loc);
#pragma clang diagnostic pop

};

}
