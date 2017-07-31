%{

#include "except.h"
#include "scanner.h"

/* Override the declaration of yylex with the one from our derived scanner. */
#ifdef YY_DECL
    #undef YY_DECL
#endif
#define YY_DECL int rumur::scanner::yylex(\
    rumur::parser::semantic_type *const lval, rumur::parser::location_type *loc)

%}

    /* Track line numbers while scanning. These are retrievable via
     * FlexLexer::lineno().
     */
%option yylineno

    /* When the end of the input is encountered, do not expect another file. */
%option noyywrap

    /* Generate a C++ scanner, not C. */
%option c++

    /* Use our own scanner class, rather than the default yyFlexLexer. */
%option yyclass="rumur::scanner"

    /* Switch to case-insensitive. */
%option caseless

    /* States for handling comments. This is not the most efficient way of
     * dealing with these, but meh.
     */
%x SC_COMMENT SC_MULTILINE_COMMENT

%%

(0x[[:xdigit:]]+|[[:digit:]]+) {
    lval->build<std::string>(YYText());
    return rumur::parser::token::NUMBER;
}

const {
    return rumur::parser::token::CONST;
}

type {
    return rumur::parser::token::TYPE;
}

var {
    return rumur::parser::token::VAR;
}

[[:alpha:][_[:alnum:]]* {
    lval->build<std::string>(YYText());
    return rumur::parser::token::ID;
}

":" {
    return rumur::parser::token::COLON;
}

"--" {
    BEGIN(SC_COMMENT);
}

"/*" {
    BEGIN(SC_MULTILINE_COMMENT);
}

    /* Ensure we keep a correct line count for error reporting. */
"\n" {
    loc->lines();
}

    /* Ignore white space. */
[[:space:]] {}

. {
    throw RumurError(std::string("unexpected character: '") + YYText() + "'");
}

    /* Comment handling. */
<SC_COMMENT>. {
    /* do nothing */
}

<SC_COMMENT>\n {
    loc->lines();
    BEGIN(INITIAL);
}

<SC_MULTILINE_COMMENT>[^*\n]* {
    /* do nothing */
}

<SC_MULTILINE_COMMENT>"*"+[^*/\n]* {
    /* do nothing */
}

<SC_MULTILINE_COMMENT>\n {
    loc->lines();
}

<SC_MULTILINE_COMMENT>"*"+"/" {
    BEGIN(INITIAL);
}
