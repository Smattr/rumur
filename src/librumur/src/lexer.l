%{

#include <rumur/except.h>
#include <rumur/scanner.h>

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

begin {
    return rumur::parser::token::BEGIN_TOK;
}

by {
    return rumur::parser::token::BY;
}

const {
    return rumur::parser::token::CONST;
}

do {
    return rumur::parser::token::DO;
}

end {
    return rumur::parser::token::END;
}

endexists {
    return rumur::parser::token::ENDEXISTS;
}

endforall {
    return rumur::parser::token::ENDFORALL;
}

endrecord {
    return rumur::parser::token::ENDRECORD;
}

endstartstate {
    return rumur::parser::token::ENDSTARTSTATE;
}

enum {
    return rumur::parser::token::ENUM;
}

exists {
    return rumur::parser::token::EXISTS;
}

forall {
    return rumur::parser::token::FORALL;
}

record {
    return rumur::parser::token::RECORD;
}

startstate {
    return rumur::parser::token::STARTSTATE;
}

to {
    return rumur::parser::token::TO;
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

"--" {
    BEGIN(SC_COMMENT);
}

"/*" {
    BEGIN(SC_MULTILINE_COMMENT);
}

":=" { return rumur::parser::token::COLON_EQ;  }
".." { return rumur::parser::token::DOTDOT;    }
">=" { return rumur::parser::token::GEQ;       }
"->" { return rumur::parser::token::IMPLIES;   }
"<=" { return rumur::parser::token::LEQ;       }
"!=" { return rumur::parser::token::NEQ;       }

[&!|:=><\-\%\+;{},\[\]\.()] { return YYText()[0]; }

\"[^"]*\" {
    lval->build<std::string>(YYText());
    return rumur::parser::token::STRING;
}

    /* Ensure we keep a correct line count for error reporting. */
"\n" {
    loc->lines();
}

    /* Ignore white space. */
[[:space:]] {}

. {
    /* FIXME: the location here is bogus */
    throw RumurError(std::string("unexpected character: '") + YYText() + "'", location());
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
