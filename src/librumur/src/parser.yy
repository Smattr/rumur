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
    #include <rumur/Expr.h>
    #include <rumur/Model.h>
    #include <rumur/Node.h>
    #include <rumur/Number.h>
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
    #include <cassert>
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

    /* And also a symbol table we'll use for relating identifiers back to the
     * target they refer to.
     */
%parse-param { rumur::Symtab<Node*> *symtab }

%token COLON_EQ
%token CONST
%token DOTDOT
%token END
%token ENDRECORD
%token ENUM
%token GEQ
%token <std::string> ID
%token IMPLIES
%token LEQ
%token NEQ
%token <std::string> NUMBER
%token RECORD
%token TYPE
%token VAR

%nonassoc COLON_EQ
%nonassoc '?' ':'
%nonassoc IMPLIES
%left '|'
%left '&'
%precedence '!'
%nonassoc '<' LEQ '=' NEQ GEQ '>'
%left '+' '-'
%left '*' '/' '%'

%type <rumur::Decl*> constdecl
%type <std::vector<rumur::Decl*>> constdecls
%type <std::vector<rumur::Decl*>> decl
%type <std::vector<rumur::Decl*>> decls
%type <rumur::ExprID*> designator
%type <rumur::Expr*> expr
%type <std::vector<std::pair<std::string, rumur::location>>> id_list;
%type <std::vector<std::pair<std::string, rumur::location>>> id_list_opt;
%type <rumur::Decl*> typedecl
%type <std::vector<rumur::Decl*>> typedecls
%type <rumur::TypeExpr*> typeexpr
%type <std::vector<rumur::VarDecl*>> vardecl
%type <std::vector<rumur::VarDecl*>> vardecls

%%

model: decls {
    output = new rumur::Model(std::move($1), @$);
};

decls: decls decl {
    $$ = $1;
    std::copy($2.begin(), $2.end(), std::back_inserter($1));
} | %empty {
    /* nothing required */
};

decl: CONST constdecls {
    $$ = $2;
} | TYPE typedecls {
    $$ = $2;
};

constdecls: constdecls constdecl {
    $$ = $1;
    $$.push_back($2);
} | %empty {
    /* nothing required */
};

constdecl: ID ':' expr ';' {
    $$ = new rumur::ConstDecl($1, $3, @$);
    symtab->declare($1, $3);
};

typedecls: typedecls typedecl {
    $$ = $1;
    $$.push_back($2);
} | %empty {
    /* nothing required */
};

typedecl: ID ':' typeexpr ';' {
    $$ = new rumur::TypeDecl($1, $3, @$);
    symtab->declare($1, $3);
};

typeexpr: ID {
    auto e = symtab->lookup<rumur::TypeExpr*>($1, @$);
    $$ = new rumur::TypeExprID($1, e, @$);
} | expr DOTDOT expr {
    $$ = new rumur::Range($1, $3, @$);
} | ENUM '{' id_list_opt '}' {
    rumur::Enum *e = new rumur::Enum($3, @$);
    /* Register all the enum members so they can be referenced later. */
    for (rumur::ExprID *eid : e->members) {
        symtab->declare(eid->id, eid);
    }
    $$ = e;
} | RECORD vardecls endrecord {
    $$ = new Record(std::move($2), @$);
};

vardecls: vardecls vardecl {
    $$ = $1;
    std::move($2.begin(), $2.end(), std::back_inserter($$));
} | %empty {
    /* nothing required */
};

vardecl: id_list_opt ':' typeexpr ';' {
    bool first = true;
    for (auto [s, l] : $1) {
        rumur::TypeExpr *t;
        if (first) {
            t = $3;
        } else {
            t = new rumur::TypeExprID("", $3, @3);
        }
        $$.emplace_back(new VarDecl(s, t, l));
        first = false;
    }
};

endrecord: END | ENDRECORD;

expr: expr '?' expr ':' expr {
    $$ = new rumur::Ternary($1, $3, $5, @$);
} | expr IMPLIES expr {
    $$ = new rumur::Implication($1, $3, @$);
} | expr '|' expr {
    $$ = new rumur::Or($1, $3, @$);
} | expr '&' expr {
    $$ = new rumur::And($1, $3, @$);
} | '!' expr {
    $$ = new rumur::Not($2, @$);
} | expr '<' expr {
    $$ = new rumur::Lt($1, $3, @$);
} | expr LEQ expr {
    $$ = new rumur::Leq($1, $3, @$);
} | expr '>' expr {
    $$ = new rumur::Gt($1, $3, @$);
} | expr GEQ expr {
    $$ = new rumur::Geq($1, $3, @$);
} | expr '=' expr {
    $$ = new rumur::Eq($1, $3, @$);
} | expr NEQ expr {
    $$ = new rumur::Neq($1, $3, @$);
} | expr '+' expr {
    $$ = new rumur::Add($1, $3, @$);
} | expr '-' expr {
    $$ = new rumur::Sub($1, $3, @$);
} | '+' expr %prec '*' {
    $$ = $2;
    $$->loc = @$;
} | '-' expr %prec '*' {
    $$ = new rumur::Negative($2, @$);
} | expr '*' expr {
    $$ = new rumur::Mul($1, $3, @$);
} | expr '/' expr {
    $$ = new rumur::Div($1, $3, @$);
} | expr '%' expr {
    $$ = new rumur::Mod($1, $3, @$);
} | designator {
    $$ = $1;
} | NUMBER {
    $$ = new rumur::Number($1, @$);
} | '(' expr ')' {
    $$ = $2;
    $$->loc = @$;
};

designator: ID {
    auto e = symtab->lookup<rumur::Expr*>($1, @$);
    assert(e != nullptr);
    $$ = new rumur::ExprID($1, e, e->type(), @$);
};

    /* Support optional trailing comma to make it easier for tools that generate
     * an input mdoels.
     */
id_list_opt: id_list comma_opt {
    $$ = $1;
} | %empty {
};

    /* Note that we create invalid ExprIDs here. We'll go back in fill in their
     * value and type_of in the rumur::Enum constructor.
     */
id_list: id_list ',' ID {
    $$ = $1;
    $$.emplace_back(std::make_pair($3, @$));
} | ID {
    $$.emplace_back(std::make_pair($1, @$));
};

comma_opt: ',' | %empty;

%%

void rumur::parser::error(const location_type &loc, const std::string &message) {
    throw RumurError(message, loc);
}
