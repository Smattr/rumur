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

  #include <cstddef>
  #include <memory>
  #include <rumur/Decl.h>
  #include <rumur/Expr.h>
  #include <rumur/Function.h>
  #include <rumur/Model.h>
  #include <rumur/Node.h>
  #include <rumur/Number.h>
  #include <rumur/Ptr.h>
  #include <rumur/Rule.h>
  #include <rumur/Stmt.h>

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
  #include <gmpxx.h>
  #include <iostream>
  #include <iterator>
  #include <rumur/except.h>
  #include <tuple>
  #include <utility>
  #include <vector>

  /* Redirect yylex to call our derived scanner. */
  #ifdef yylex
    #undef yylex
  #endif
  #define yylex sc.yylex

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
%parse-param { scanner &sc }

  /* Tell Bison we'll receive another parameter that will allow us to pass
   * back the result of parsing.
   */
%parse-param { rumur::Ptr<rumur::Model> &output }

%token ALIAS
%token ARRAY
%token ARROW
%token ASSERT
%token ASSUME
%token BEGIN_TOK
%token BOOLEAN
%token BY
%token CLEAR
%token COLON_EQ
%token CONST
%token DEQ
%token DO
%token DOTDOT
%token ELSE
%token ELSIF
%token END
%token ENDALIAS
%token ENDEXISTS
%token ENDFOR
%token ENDFORALL
%token ENDFUNCTION
%token ENDIF
%token ENDPROCEDURE
%token ENDRECORD
%token ENDRULE
%token ENDRULESET
%token ENDSTARTSTATE
%token ENUM
%token ERROR
%token EXISTS
%token FOR
%token FORALL
%token FUNCTION
%token GEQ
%token <std::string> ID
%token IF
%token IMPLIES
%token INVARIANT
%token LEQ
%token NEQ
%token <std::string> NUMBER
%token OF
%token PROCEDURE
%token PROPERTY
%token RECORD
%token RETURN
%token RULE
%token RULESET
%token SCALARSET
%token STARTSTATE
%token <std::string> STRING
%token THEN
%token TO
%token TYPE
%token UNDEFINE
%token VAR

%nonassoc '?' ':'
%nonassoc IMPLIES
%left '|'
%left '&'
%precedence '!'
%nonassoc '<' LEQ DEQ '=' NEQ GEQ '>'
%left '+' '-'
%left '*' '/' '%'

%type <std::shared_ptr<rumur::AliasRule>>                    aliasrule
%type <std::shared_ptr<rumur::Property::Category>>           category
%type <std::vector<std::shared_ptr<rumur::Decl>>>            decl
%type <std::vector<std::shared_ptr<rumur::Decl>>>            decls
%type <std::vector<std::shared_ptr<rumur::Decl>>>            decls_header
%type <std::shared_ptr<rumur::Expr>>                         designator
%type <std::vector<rumur::IfClause>>                         else_opt
%type <std::vector<rumur::IfClause>>                         elsifs
%type <std::shared_ptr<rumur::Expr>>                         expr
%type <std::vector<std::tuple<std::string, std::shared_ptr<rumur::Expr>, rumur::location>>> exprdecl
%type <std::vector<std::tuple<std::string, std::shared_ptr<rumur::Expr>, rumur::location>>> exprdecls
%type <std::vector<std::shared_ptr<rumur::Expr>>>            exprlist
%type <std::vector<std::shared_ptr<rumur::Expr>>>            exprlist_cont
%type <std::shared_ptr<rumur::Expr>>                         guard_opt
%type <std::vector<std::pair<std::string, rumur::location>>> id_list
%type <std::vector<std::pair<std::string, rumur::location>>> id_list_opt
%type <std::vector<std::shared_ptr<rumur::VarDecl>>>         parameter
%type <std::vector<std::shared_ptr<rumur::VarDecl>>>         parameters
%type <std::shared_ptr<rumur::Function>>                     procdecl
%type <std::vector<std::shared_ptr<rumur::Function>>>        procdecls
%type <std::shared_ptr<rumur::PropertyRule>>                 property
%type <std::shared_ptr<rumur::Quantifier>>                   quantifier
%type <std::vector<rumur::Quantifier>>                       quantifiers
%type <std::shared_ptr<rumur::TypeExpr>>                     return_type
%type <std::shared_ptr<rumur::Rule>>                         rule
%type <std::shared_ptr<rumur::Ruleset>>                      ruleset
%type <std::vector<std::shared_ptr<rumur::Rule>>>            rules
%type <std::shared_ptr<rumur::SimpleRule>>                   simplerule
%type <std::shared_ptr<rumur::StartState>>                   startstate
%type <rumur::Ptr<rumur::Stmt>>                              stmt
%type <std::vector<rumur::Ptr<rumur::Stmt>>>                 stmts
%type <std::vector<rumur::Ptr<rumur::Stmt>>>                 stmts_cont
%type <std::string>                                          string_opt
%type <std::vector<std::shared_ptr<rumur::Decl>>>            typedecl
%type <std::vector<std::shared_ptr<rumur::Decl>>>            typedecls
%type <std::shared_ptr<rumur::TypeExpr>>                     typeexpr
%type <std::vector<std::shared_ptr<rumur::VarDecl>>>         vardecl
%type <std::vector<std::shared_ptr<rumur::VarDecl>>>         vardecls
%type <std::shared_ptr<bool>>                                var_opt

%%

model: decls procdecls rules {
  output = rumur::Ptr<rumur::Model>::make(std::move($1), std::move($2), std::move($3), @$);
};

aliasrule: ALIAS exprdecls DO rules endalias {
  std::vector<std::shared_ptr<rumur::AliasDecl>> decls;
  for (const std::tuple<std::string, std::shared_ptr<rumur::Expr>, rumur::location> &d : $2) {
    decls.push_back(std::make_shared<rumur::AliasDecl>(std::get<0>(d), std::get<1>(d), std::get<2>(d)));
  }
  $$ = std::make_shared<rumur::AliasRule>(std::move(decls), std::move($4), @$);
};

begin_opt: BEGIN_TOK | %empty;

category: ASSERT {
  $$ = std::make_shared<rumur::Property::Category>(rumur::Property::ASSERTION);
} | ASSUME {
  $$ = std::make_shared<rumur::Property::Category>(rumur::Property::ASSUMPTION);
} | INVARIANT {
  $$ = std::make_shared<rumur::Property::Category>(rumur::Property::ASSERTION);
} | PROPERTY {
  $$ = std::make_shared<rumur::Property::Category>(rumur::Property::DISABLED);
};

comma_opt: ',' | %empty;

decl: CONST exprdecls {
  for (const std::tuple<std::string, std::shared_ptr<rumur::Expr>, rumur::location> &d : $2) {
    $$.push_back(std::make_shared<rumur::ConstDecl>(std::get<0>(d), std::get<1>(d), std::get<2>(d)));
  }
} | TYPE typedecls {
  $$ = $2;
} | VAR vardecls {
  std::move($2.begin(), $2.end(), std::back_inserter($$));
};

decls: decls decl {
  $$ = $1;
  std::move($2.begin(), $2.end(), std::back_inserter($$));
} | %empty {
  /* nothing required */
};

decls_header: decls BEGIN_TOK {
  $$ = $1;
} | %empty {
};

designator: designator '.' ID {
  $$ = std::make_shared<rumur::Field>($1, $3, @$);
} | designator '[' expr ']' {
  $$ = std::make_shared<rumur::Element>($1, $3, @$);
} | ID {
  $$ = std::make_shared<rumur::ExprID>($1, nullptr, @$);
};

else_opt: ELSE stmts {
  $$.push_back(rumur::IfClause(std::shared_ptr<Expr>(), $2, @$));
} | %empty {
};

elsifs: elsifs ELSIF expr THEN stmts {
  $$ = $1;
  $$.push_back(rumur::IfClause($3, $5, rumur::location(@2.begin, @5.end)));
} | %empty {
};

endalias: END | ENDALIAS;
endexists: END | ENDEXISTS;
endfor: END | ENDFOR;
endforall: END | ENDFORALL;
endfunction: END | ENDFUNCTION | ENDPROCEDURE;
endif: END | ENDIF;
endrecord: END | ENDRECORD;
endrule: END | ENDRULE;
endruleset: END | ENDRULESET;
endstartstate: END | ENDSTARTSTATE;

expr: expr '?' expr ':' expr {
  $$ = std::make_shared<rumur::Ternary>($1, $3, $5, @$);
} | expr IMPLIES expr {
  $$ = std::make_shared<rumur::Implication>($1, $3, @$);
} | expr '|' expr {
  $$ = std::make_shared<rumur::Or>($1, $3, @$);
} | expr '&' expr {
  $$ = std::make_shared<rumur::And>($1, $3, @$);
} | '!' expr {
  $$ = std::make_shared<rumur::Not>($2, @$);
} | expr '<' expr {
  $$ = std::make_shared<rumur::Lt>($1, $3, @$);
} | expr LEQ expr {
  $$ = std::make_shared<rumur::Leq>($1, $3, @$);
} | expr '>' expr {
  $$ = std::make_shared<rumur::Gt>($1, $3, @$);
} | expr GEQ expr {
  $$ = std::make_shared<rumur::Geq>($1, $3, @$);
} | expr DEQ expr {
  $$ = std::make_shared<rumur::Eq>($1, $3, @$);
} | expr '=' expr {
  $$ = std::make_shared<rumur::Eq>($1, $3, @$);
} | expr NEQ expr {
  $$ = std::make_shared<rumur::Neq>($1, $3, @$);
} | expr '+' expr {
  $$ = std::make_shared<rumur::Add>($1, $3, @$);
} | expr '-' expr {
  $$ = std::make_shared<rumur::Sub>($1, $3, @$);
} | '+' expr %prec '*' {
  $$ = $2;
  $$->loc = @$;
} | '-' expr %prec '*' {
  $$ = std::make_shared<rumur::Negative>($2, @$);
} | expr '*' expr {
  $$ = std::make_shared<rumur::Mul>($1, $3, @$);
} | expr '/' expr {
  $$ = std::make_shared<rumur::Div>($1, $3, @$);
} | expr '%' expr {
  $$ = std::make_shared<rumur::Mod>($1, $3, @$);
} | FORALL quantifier DO expr endforall {
    $$ = std::make_shared<rumur::Forall>(*$2, $4, @$);
} | EXISTS quantifier DO expr endexists {
    $$ = std::make_shared<rumur::Exists>(*$2, $4, @$);
} | designator {
  $$ = $1;
} | NUMBER {
  $$ = std::make_shared<rumur::Number>($1, @$);
} | '(' expr ')' {
  $$ = $2;
  $$->loc = @$;
} | ID '(' exprlist ')' {
  $$ = std::make_shared<rumur::FunctionCall>($1, nullptr, std::move($3), @$);
};

exprdecl: id_list_opt ':' expr {
  for (const std::pair<std::string, rumur::location> &m : $1) {
    $$.push_back(std::make_tuple(m.first, $3, @$));
  }
};

exprdecls: exprdecls exprdecl semi_opt {
  $$ = $1;
  std::move($2.begin(), $2.end(), std::back_inserter($$));
} | %empty {
  /* nothing required */
};

exprlist: exprlist_cont expr comma_opt {
  $$ = $1;
  $$.push_back($2);
} | %empty {
};

exprlist_cont: exprlist_cont expr ',' {
  $$ = $1;
  $$.push_back($2);
} | %empty {
};

function: FUNCTION | PROCEDURE;

guard_opt: expr ARROW {
  $$ = $1;
} | %empty {
  $$ = nullptr;
};

id_list: id_list ',' ID {
  $$ = $1;
  $$.emplace_back(std::make_pair($3, @3));
} | ID {
  $$.emplace_back(std::make_pair($1, @$));
};

  /* Support optional trailing comma to make it easier for tools that generate
   * an input mdoels.
   */
id_list_opt: id_list comma_opt {
  $$ = $1;
} | %empty {
};

parameter: var_opt id_list ':' typeexpr {
  for (const std::pair<std::string, rumur::location> &i : $2) {
    auto v = std::make_shared<rumur::VarDecl>(i.first, $4, @$);
    v->readonly = !*$1;
    $$.push_back(v);
  }
};

parameters: parameters parameter semi_opt {
  $$ = $1;
  std::move($2.begin(), $2.end(), std::back_inserter($$));
} | %empty {
};

procdecl: function ID '(' parameters ')' return_type decls begin_opt stmts endfunction semi_opt {
  $$ = std::make_shared<rumur::Function>($2, std::move($4), $6, std::move($7), $9, @$);
};

procdecls: procdecls procdecl {
  $$ = $1;
  $$.push_back($2);
} | %empty {
};

property: category STRING expr {
  rumur::Property p(*$1, $3, @3);
  $$ = std::make_shared<rumur::PropertyRule>($2, p, @$);
} | category expr string_opt {
  rumur::Property p(*$1, $2, @2);
  $$ = std::make_shared<rumur::PropertyRule>($3, p, @$);
};

quantifier: ID ':' typeexpr {
  $$ = std::make_shared<rumur::Quantifier>($1, $3, @$);
} | ID COLON_EQ expr TO expr BY expr {
  $$ = std::make_shared<rumur::Quantifier>($1, $3, $5, $7, @$);
} | ID COLON_EQ expr TO expr {
  $$ = std::make_shared<rumur::Quantifier>($1, $3, $5, @$);
};

quantifiers: quantifiers ';' quantifier {
  $$ = $1;
  $$.push_back(*$3);
} | quantifier {
  $$.push_back(*$1);
};

return_type: ':' typeexpr semi_opt {
  $$ = $2;
} | semi_opt {
  $$ = nullptr;
};

rule: startstate {
  $$ = $1;
} | simplerule {
  $$ = $1;
} | property {
  $$ = $1;
} | ruleset {
  $$ = $1;
} | aliasrule {
  $$ = $1;
};

rules: rules rule semi_opt {
  $$ = $1;
  $$.push_back($2);
} | %empty {
};

ruleset: RULESET quantifiers DO rules endruleset {
  $$ = std::make_shared<rumur::Ruleset>($2, std::move($4), @$);
};

semi_opt: ';' | %empty;

simplerule: RULE string_opt guard_opt decls_header stmts endrule {
  $$ = std::make_shared<rumur::SimpleRule>($2, $3, std::move($4), $5, @$);
};

startstate: STARTSTATE string_opt decls_header stmts endstartstate {
  $$ = std::make_shared<rumur::StartState>($2, std::move($3), $4, @$);
};

stmt: category STRING expr {
  rumur::Property p(*$1, $3, @3);
  $$ = rumur::Ptr<rumur::PropertyStmt>::make(p, $2, @$);
} | category expr string_opt {
  rumur::Property p(*$1, $2, @2);
  $$ = rumur::Ptr<rumur::PropertyStmt>::make(p, $3, @$);
} | designator COLON_EQ expr {
  $$ = rumur::Ptr<rumur::Assignment>::make($1, $3, @$);
} | ALIAS exprdecls DO stmts endalias {
  std::vector<std::shared_ptr<rumur::AliasDecl>> decls;
  for (const std::tuple<std::string, std::shared_ptr<rumur::Expr>, rumur::location> &d : $2) {
    decls.push_back(std::make_shared<rumur::AliasDecl>(std::get<0>(d), std::get<1>(d), std::get<2>(d)));
  }
  $$ = rumur::Ptr<rumur::AliasStmt>::make(std::move(decls), $4, @$);
} | ERROR STRING {
  $$ = rumur::Ptr<rumur::ErrorStmt>::make($2, @$);
} | CLEAR designator {
  $$ = rumur::Ptr<rumur::Clear>::make($2, @$);
} | FOR quantifier DO stmts endfor {
  $$ = rumur::Ptr<rumur::For>::make(*$2, $4, @$);
} | IF expr THEN stmts elsifs else_opt endif {
  std::vector<rumur::IfClause> cs = {
    rumur::IfClause($2, $4, rumur::location(@1.begin, @4.end)) };
  cs.insert(cs.end(), $5.begin(), $5.end());
  cs.insert(cs.end(), $6.begin(), $6.end());
  $$ = rumur::Ptr<rumur::If>::make(std::move(cs), @$);
} | RETURN {
  $$ = rumur::Ptr<rumur::Return>::make(std::shared_ptr<Expr>(), @$);
} | RETURN expr {
  $$ = rumur::Ptr<rumur::Return>::make($2, @$);
} | UNDEFINE designator {
  $$ = rumur::Ptr<rumur::Undefine>::make($2, @$);
} | ID '(' exprlist ')' {
  $$ = rumur::Ptr<rumur::ProcedureCall>::make($1, nullptr, std::move($3), @$);
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

string_opt: STRING {
  $$ = $1;
} | %empty {
  /* nothing required */
};

typedecl: id_list_opt ':' typeexpr {
  for (const std::pair<std::string, rumur::location> &m : $1) {
    $$.push_back(std::make_shared<rumur::TypeDecl>(m.first, $3, @$));
  }
};

typedecls: typedecls typedecl semi_opt {
  $$ = $1;
  std::move($2.begin(), $2.end(), std::back_inserter($$));
} | %empty {
  /* nothing required */
};

typeexpr: BOOLEAN {
  /* We need to special case this instead of just using the ID rule because IDs
   * are treated as case-sensitive while "boolean" is not. To avoid awkwardness
   * in later symbol resolution, we force it to lower case here.
   */
  $$ = std::make_shared<rumur::TypeExprID>("boolean", nullptr, @$);
} | ID {
  $$ = std::make_shared<rumur::TypeExprID>($1, nullptr, @$);
} | expr DOTDOT expr {
  $$ = std::make_shared<rumur::Range>($1, $3, @$);
} | ENUM '{' id_list_opt '}' {
  $$ = std::make_shared<rumur::Enum>(std::move($3), @$);
} | RECORD vardecls endrecord {
  $$ = std::make_shared<rumur::Record>(std::move($2), @$);
} | ARRAY '[' typeexpr ']' OF typeexpr {
  $$ = std::make_shared<rumur::Array>($3, $6, @$);
} | SCALARSET '(' expr ')' {
  $$ = std::make_shared<rumur::Scalarset>($3, @$);
};

vardecl: id_list_opt ':' typeexpr {
  for (const std::pair<std::string, rumur::location> &m : $1) {
    $$.push_back(std::make_shared<rumur::VarDecl>(m.first, $3, @$));
  }
};

vardecls: vardecls vardecl semi_opt {
  $$ = $1;
  std::move($2.begin(), $2.end(), std::back_inserter($$));
} | %empty {
  /* nothing required */
};

var_opt: VAR {
  $$ = std::make_shared<bool>(true);
} | %empty {
  $$ = std::make_shared<bool>(false);
};

%%

void rumur::parser::error(const location_type &loc, const std::string &message) {
  throw Error(message, loc);
}
