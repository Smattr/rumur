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
    #include <rumur/Indexer.h>
    #include <rumur/Model.h>
    #include <rumur/Node.h>
    #include <rumur/Number.h>
    #include <rumur/Rule.h>
    #include <rumur/Stmt.h>
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
    #include <memory>
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
%parse-param { rumur::Symtab<Node> *symtab }

    /* A helper for generating unique IDs for nodes that require it. */
%parse-param { rumur::Indexer &indexer }

%token BEGIN_TOK
%token BY
%token COLON_EQ
%token CONST
%token DEQ
%token DO
%token DOTDOT
%token END
%token ENDEXISTS
%token ENDFORALL
%token ENDRECORD
%token ENDSTARTSTATE
%token ENUM
%token EXISTS
%token FORALL
%token GEQ
%token <std::string> ID
%token IMPLIES
%token LEQ
%token NEQ
%token <std::string> NUMBER
%token RECORD
%token STARTSTATE
%token <std::string> STRING
%token TO
%token TYPE
%token VAR

%nonassoc '?' ':'
%nonassoc IMPLIES
%left '|'
%left '&'
%precedence '!'
%nonassoc '<' LEQ DEQ '=' NEQ GEQ '>'
%left '+' '-'
%left '*' '/' '%'

%type <std::shared_ptr<rumur::Decl>> constdecl
%type <std::vector<std::shared_ptr<rumur::Decl>>> constdecls
%type <std::vector<std::shared_ptr<rumur::Decl>>> decl
%type <std::vector<std::shared_ptr<rumur::Decl>>> decls
%type <std::vector<std::shared_ptr<rumur::Decl>>> decls_header
%type <std::shared_ptr<rumur::Lvalue>> designator
%type <std::shared_ptr<rumur::Expr>> expr
%type <std::vector<std::pair<std::string, rumur::location>>> id_list
%type <std::vector<std::pair<std::string, rumur::location>>> id_list_opt
%type <std::shared_ptr<rumur::Quantifier>> quantifier
%type <std::shared_ptr<rumur::Rule>> rule
%type <std::vector<std::shared_ptr<rumur::Rule>>> rules
%type <std::vector<std::shared_ptr<rumur::Rule>>> rules_cont
%type <std::shared_ptr<rumur::StartState>> startstate
%type <std::shared_ptr<rumur::Stmt>> stmt
%type <std::vector<std::shared_ptr<rumur::Stmt>>> stmts
%type <std::vector<std::shared_ptr<rumur::Stmt>>> stmts_cont
%type <std::string> string_opt
%type <std::shared_ptr<rumur::Decl>> typedecl
%type <std::vector<std::shared_ptr<rumur::Decl>>> typedecls
%type <std::shared_ptr<rumur::TypeExpr>> typeexpr
%type <std::vector<std::shared_ptr<rumur::VarDecl>>> vardecl
%type <std::vector<std::shared_ptr<rumur::VarDecl>>> vardecls

%%

model: decls rules {
    output = new rumur::Model(std::move($1), std::move($2), @$, indexer);
};

decls: decls decl {
    $$ = $1;
    std::copy($2.begin(), $2.end(), std::back_inserter($$));
} | %empty {
    /* nothing required */
};

decl: CONST constdecls {
    $$ = $2;
} | TYPE typedecls {
    $$ = $2;
} | VAR vardecls {
    for (std::shared_ptr<rumur::VarDecl> d : $2) {
        symtab->declare(d->name, std::make_shared<rumur::Var>(d, d->loc, indexer));
    }
    std::move($2.begin(), $2.end(), std::back_inserter($$));
};

constdecls: constdecls constdecl {
    $$ = $1;
    $$.push_back($2);
} | %empty {
    /* nothing required */
};

constdecl: ID ':' expr ';' {
    $$ = std::make_shared<rumur::ConstDecl>($1, $3, @$, indexer);
    symtab->declare($1, $3);
};

typedecls: typedecls typedecl {
    $$ = $1;
    $$.push_back($2);
} | %empty {
    /* nothing required */
};

typedecl: ID ':' typeexpr ';' {
    $$ = std::make_shared<rumur::TypeDecl>($1, $3, @$, indexer);
    symtab->declare($1, $3);
};

typeexpr: ID {
    $$ = symtab->lookup<rumur::TypeExpr>($1, @$);
} | expr DOTDOT expr {
    $$ = std::make_shared<rumur::Range>($1, $3, @$, indexer);
} | ENUM '{' id_list_opt '}' {
    std::shared_ptr<rumur::Enum> e = std::make_shared<rumur::Enum>($3, @$, indexer);
    /* Register all the enum members so they can be referenced later. */
    for (std::shared_ptr<rumur::ExprID> eid : e->members) {
        symtab->declare(eid->id, eid);
    }
    $$ = e;
} | RECORD vardecls endrecord {
    $$ = std::make_shared<rumur::Record>(std::move($2), @$, indexer);
};

vardecls: vardecls vardecl {
    $$ = $1;
    std::move($2.begin(), $2.end(), std::back_inserter($$));
} | %empty {
    /* nothing required */
};

vardecl: id_list_opt ':' typeexpr ';' {
    for (auto [s, l] : $1) {
        $$.push_back(std::make_shared<rumur::VarDecl>(s, $3, l, indexer));
    }
};

endrecord: END | ENDRECORD;

rules: rules_cont rule semi_opt {
    $$ = $1;
    $$.push_back($2);
} | rule semi_opt {
    $$.push_back($1);
} | %empty {
};

rules_cont: rules_cont rule ';' {
    $$ = $1;
    $$.push_back($2);
} | rule ';' {
    $$.push_back($1);
};

rule: startstate {
    $$ = $1;
};

startstate: STARTSTATE string_opt decls_header stmts endstartstate {
    $$ = std::make_shared<rumur::StartState>($2, std::move($3), std::move($4), @$, indexer);
};

decls_header: decls BEGIN_TOK {
    $$ = $1;
} | %empty {
};

stmts: stmts_cont stmt semi_opt {
    $$ = $1;
    $$.push_back($2);
} | stmt semi_opt {
    $$.push_back($1);
} | %empty {
};

stmts_cont: stmts_cont stmt ';' {
    $$ = $1;
    $$.push_back($2);
} | stmt ';' {
    $$.push_back($1);
};

stmt: designator COLON_EQ expr {
    $$ = std::make_shared<rumur::Assignment>($1, $3, @$, indexer);
};

endstartstate: END | ENDSTARTSTATE;

string_opt: STRING {
    $$ = $1.substr(1, $1.size() - 2); /* strip quotes */
} | %empty {
    /* nothing required */
};

semi_opt: ';' | %empty;

expr: expr '?' expr ':' expr {
    $$ = std::make_shared<rumur::Ternary>($1, $3, $5, @$, indexer);
} | expr IMPLIES expr {
    $$ = std::make_shared<rumur::Implication>($1, $3, @$, indexer);
} | expr '|' expr {
    $$ = std::make_shared<rumur::Or>($1, $3, @$, indexer);
} | expr '&' expr {
    $$ = std::make_shared<rumur::And>($1, $3, @$, indexer);
} | '!' expr {
    $$ = std::make_shared<rumur::Not>($2, @$, indexer);
} | expr '<' expr {
    $$ = std::make_shared<rumur::Lt>($1, $3, @$, indexer);
} | expr LEQ expr {
    $$ = std::make_shared<rumur::Leq>($1, $3, @$, indexer);
} | expr '>' expr {
    $$ = std::make_shared<rumur::Gt>($1, $3, @$, indexer);
} | expr GEQ expr {
    $$ = std::make_shared<rumur::Geq>($1, $3, @$, indexer);
} | expr DEQ expr {
    $$ = std::make_shared<rumur::Eq>($1, $3, @$, indexer);
} | expr '=' expr {
    $$ = std::make_shared<rumur::Eq>($1, $3, @$, indexer);
} | expr NEQ expr {
    $$ = std::make_shared<rumur::Neq>($1, $3, @$, indexer);
} | expr '+' expr {
    $$ = std::make_shared<rumur::Add>($1, $3, @$, indexer);
} | expr '-' expr {
    $$ = std::make_shared<rumur::Sub>($1, $3, @$, indexer);
} | '+' expr %prec '*' {
    $$ = $2;
    $$->loc = @$;
} | '-' expr %prec '*' {
    $$ = std::make_shared<rumur::Negative>($2, @$, indexer);
} | expr '*' expr {
    $$ = std::make_shared<rumur::Mul>($1, $3, @$, indexer);
} | expr '/' expr {
    $$ = std::make_shared<rumur::Div>($1, $3, @$, indexer);
} | expr '%' expr {
    $$ = std::make_shared<rumur::Mod>($1, $3, @$, indexer);
} | FORALL quantifier {
        symtab->open_scope();
        symtab->declare($2->var->name, std::make_shared<rumur::Var>($2->var, $2->loc, indexer));
    } DO expr endforall {
        $$ = std::make_shared<rumur::Forall>($2, $5, @$, indexer);
        symtab->close_scope();
} | EXISTS quantifier {
        symtab->open_scope();
        symtab->declare($2->var->name, std::make_shared<rumur::Var>($2->var, $2->loc, indexer));
    } DO expr endexists {
        $$ = std::make_shared<rumur::Exists>($2, $5, @$, indexer);
        symtab->close_scope();
} | designator {
    $$ = $1;
} | NUMBER {
    $$ = std::make_shared<rumur::Number>($1, @$, indexer);
} | '(' expr ')' {
    $$ = $2;
    $$->loc = @$;
};

quantifier: ID ':' typeexpr {
    $$ = std::make_shared<rumur::Quantifier>($1, $3, @$, indexer);
} | ID ':' expr TO expr BY expr {
    $$ = std::make_shared<rumur::Quantifier>($1, $3, $5, $7, @$, indexer);
} | ID ':' expr TO expr {
    $$ = std::make_shared<rumur::Quantifier>($1, $3, $5, @$, indexer);
};

designator: designator '.' ID {
    $$ = std::make_shared<rumur::Field>($1, $3, @$, indexer);
} | designator '[' expr ']' {
    $$ = std::make_shared<rumur::Element>($1, $3, @$, indexer);
} | ID {
    auto e = symtab->lookup<rumur::Expr>($1, @$);
    assert(e != nullptr);
    $$ = std::make_shared<rumur::ExprID>($1, e, e->type(), @$, indexer);
};

endforall: END | ENDFORALL;

endexists: END | ENDEXISTS;

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
