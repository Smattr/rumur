%skeleton "lalr1.cc"
%require "3.0"

    /* Ordinarily, Bison emits the parser in a namespace "yy". By changing the
     * namespace our parser can coexist with other Bison-generated parsers if
     * necessary.
     */
%define api.namespace {rumur}

    /* Make yylval, the semantic type of the current token, a variant. The
     * purpose of this is to allow us to return richer information about the
     * contents of a token or expression.
     */
%define api.value.type variant

    /* Turn on some safety checks for construction and destruction of variant
     * types. This is only relevant if we enable them (api.value.type variant).
     */
%define parse.assert

    /* Receive source location from the scanner. This allows us to give proper
     * feedback on user errors, pointing them at the line and column where they
     * went wrong.
     */
%locations

    /* Code that we need before anything in both the implementation
     * (parser.yy.cc) and the header (parser.yy.hh).
     */
%code requires {

    #include <rumur/Decl.h>
    #include <rumur/Model.h>
    #include <rumur/Symtab.h>

    /* Forward declare the scanner class that Flex will produce for us. */
    namespace rumur {
        class scanner;
    }

}

    /* Code that we need in the implementation, but in no particular location.
     */
%code {

    #include <algorithm>
    #include <iostream>
    #include <iterator>
    #include <utility>
    #include <vector>

    /* Redirect yylex to call our derived scanner. */
    #ifdef yylex
        #undef yylex
    #endif
    #define yylex s.yylex

}

    /* Code that we need to include at the top of the implementation
     * (parser.yy.cc) but not in the header. Including this code in the header
     * (parser.yy.hh) would induce a circular include.
     */
%code top {

    #include <rumur/scanner.h>

}

    /* Tell Bison that the parser receives a reference to an instance of our
     * scanner class, in order to use our redirected yylex defined above.
     */
%parse-param { scanner &s }

    /* Tell Bison we'll receive another parameter that will allow us to pass
     * back the result of parsing.
     */
%parse-param { rumur::Model *&output }

%token COLON_EQ
%token CONST
%token GEQ
%token <std::string> ID
%token IMPLIES
%token LEQ
%token NEQ
%token <std::string> NUMBER
%token TYPE
%token VAR

%nonassoc COLON_EQ
%nonassoc '?' ':'
%nonassoc IMPLIES
%left '|'
%left '&'
%left '!'
%nonassoc '<' LEQ '=' NEQ GEQ '>'
%left '+' '-'
%left '*' '/' '%'

%type <rumur::Decl*> constdecl
%type <std::vector<rumur::Decl*>> constdecls
%type <std::vector<rumur::Decl*>> decl
%type <std::vector<rumur::Decl*>> decls
%type <rumur::Number*> number

%%

model: decls {
    output = new Model(std::move($1));
};

decls: decls decl {
    $$ = $1;
    std::copy($2.begin(), $2.end(), std::back_inserter($1));
} | {
    /* nothing required */
};

decl: CONST constdecls {
    $$ = $2;
};

constdecls: constdecls constdecl {
    $$ = $1;
    $$.push_back($2);
} | {
    /* nothing required */
};

constdecl: ID ':' number ';' {
    $$ = new rumur::ConstDecl($1, $3);
};

number: NUMBER {
    $$ = new rumur::Number($1);
}

%%

void rumur::parser::error(const location_type &loc, const std::string &message) {
    std::cerr << "Error: " << message << " at " << loc << "\n";
}
