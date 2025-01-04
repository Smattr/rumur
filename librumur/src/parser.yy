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

  /* Enable more explanatory error messages. Without this, Bison just says
   * "syntax error" for any problem. The extra information can sometimes be
   * inaccurate unless you define "parse.lac full" which is not available for
   * C++, but even a slightly inaccurate message is typically more useful to the
   * user than "syntax error".
   */
%define parse.error verbose

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

  /* squash warnings that Bison-generated code triggers under Clang ≥14 */
  #ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wunused-but-set-variable"
  #endif
}

  /* Tell Bison that the parser receives a reference to an instance of our
   * scanner class, in order to use our redirected yylex defined above.
   */
%parse-param { scanner &sc }

  /* Tell Bison we'll receive another parameter that will allow us to pass
   * back the result of parsing.
   */
%parse-param { rumur::Ptr<rumur::Node> &output }

  /* Tell Bison our parser should have an extra class member that will be passed
   * during construction, which should in turn be passed on to the scanner
   * during lexing.
   */
%lex-param { int &start_token }
%parse-param { int start_token }

%token ALIAS
%token AMPAMP "&&"
%token ARRAY
%token ARROW
%token ASSERT
%token ASSUME
%token BEGIN_TOK
%token BOOLEAN
%token BY
%token CASE
%token CLEAR
%token COLON_EQ ":="
%token CONST
%token COVER
%token DEQ "=="
%token DO
%token DOTDOT ".."
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
%token ENDSWITCH
%token ENDWHILE
%token ENUM
%token ERROR
%token EXISTS
%token FOR
%token FORALL
%token FUNCTION
%token GEQ ">="
%token <std::string> ID
%token IF
%token IMPLIES "->"
%token INVARIANT
%token ISUNDEFINED
%token LAND "∧"
%token LEQ "<="
%token LIVENESS
%token LOR "∨"
%token LSH "<<"
%token NEQ "!="
%token <std::string> NUMBER
%token OF
%token PIPEPIPE "||"
%token PROCEDURE
%token PUT
%token RECORD
%token RETURN
%token RSH ">>"
%token RULE
%token RULESET
%token SCALARSET
%token START_DECL
%token START_EXPR
%token START_MODEL
%token START_PROPERTY
%token START_RULE
%token START_STMT
%token STARTSTATE
%token <std::string> STRING
%token SWITCH
%token THEN
%token TO
%token TYPE
%token UNDEFINE
%token VAR
%token WHILE

%nonassoc '?' ':'
%nonassoc IMPLIES
%left PIPEPIPE LOR
%left AMPAMP LAND
%left '|'
%left '^'
%left '&'
%precedence '!'
%nonassoc '<' LEQ DEQ '=' NEQ GEQ '>'
%left LSH RSH
%left '+' '-'
%left '*' '/' '%'
%precedence '~'

%type <rumur::Ptr<rumur::AliasRule>>                         aliasrule
%type <std::shared_ptr<rumur::Property::Category>>           category
%type <std::vector<rumur::Ptr<rumur::Decl>>>                 decl
%type <std::vector<rumur::Ptr<rumur::Decl>>>                 decls
%type <std::vector<rumur::Ptr<rumur::Decl>>>                 decls_header
%type <rumur::Ptr<rumur::Expr>>                              designator
%type <std::vector<rumur::IfClause>>                         else_opt
%type <std::vector<rumur::IfClause>>                         elsifs
%type <rumur::Ptr<rumur::Expr>>                              expr
%type <std::vector<std::tuple<std::string, rumur::Ptr<rumur::Expr>, rumur::location>>> exprdecl
%type <std::vector<std::tuple<std::string, rumur::Ptr<rumur::Expr>, rumur::location>>> exprdecls
%type <std::vector<rumur::Ptr<rumur::Expr>>>                 exprlist
%type <std::vector<rumur::Ptr<rumur::Expr>>>                 exprlist_cont
%type <rumur::Ptr<rumur::Expr>>                              guard_opt
%type <std::vector<std::pair<std::string, rumur::location>>> id_list
%type <std::vector<std::pair<std::string, rumur::location>>> id_list_opt
%type <rumur::Ptr<rumur::Model>>                             model
%type <std::vector<rumur::Ptr<rumur::Node>>>                 nodes
%type <std::vector<rumur::Ptr<rumur::VarDecl>>>              parameter
%type <std::vector<rumur::Ptr<rumur::VarDecl>>>              parameters
%type <rumur::Ptr<rumur::Function>>                          procdecl
%type <rumur::Ptr<rumur::PropertyRule>>                      property
%type <std::shared_ptr<rumur::Quantifier>>                   quantifier
%type <std::vector<rumur::Quantifier>>                       quantifiers
%type <rumur::Ptr<rumur::TypeExpr>>                          return_type
%type <rumur::Ptr<rumur::Rule>>                              rule
%type <rumur::Ptr<rumur::Ruleset>>                           ruleset
%type <std::vector<rumur::Ptr<rumur::Rule>>>                 rules
%type <rumur::Ptr<rumur::SimpleRule>>                        simplerule
%type <rumur::Ptr<rumur::StartState>>                        startstate
%type <rumur::Ptr<rumur::Stmt>>                              stmt
%type <std::vector<rumur::Ptr<rumur::Stmt>>>                 stmts
%type <std::vector<rumur::Ptr<rumur::Stmt>>>                 stmts_cont
%type <std::string>                                          string_opt
%type <std::vector<rumur::SwitchCase>>                       switchcases
%type <std::vector<rumur::SwitchCase>>                       switchcases_cont
%type <std::vector<rumur::Ptr<rumur::Decl>>>                 typedecl
%type <std::vector<rumur::Ptr<rumur::Decl>>>                 typedecls
%type <rumur::Ptr<rumur::TypeExpr>>                          typeexpr
%type <std::vector<rumur::Ptr<rumur::VarDecl>>>              vardecl
%type <std::vector<rumur::Ptr<rumur::VarDecl>>>              vardecls
%type <std::shared_ptr<bool>>                                var_opt

%%

start:
    START_DECL decl {
  if ($2.size() != 1) {
    throw rumur::Error("only parsing a single declaration is supported", rumur::location());
  }
  output = $2[0];
} | START_EXPR expr {
  output = $2;
} | START_MODEL model {
  output = $2;
} | START_PROPERTY property {
  output = $2;
} | START_RULE rule {
  output = $2;
} | START_STMT stmt {
  output = $2;
};

model: nodes {
  $$ = rumur::Ptr<rumur::Model>::make($1, @$);
};

nodes: nodes decl {
  $$ = $1;
  std::move($2.begin(), $2.end(), std::back_inserter($$));
} | nodes procdecl {
  $$ = $1;
  $$.push_back($2);
} | nodes rule semi_opt {
  $$ = $1;
  $$.push_back($2);
} | %empty {
};

aliasrule: ALIAS exprdecls DO rules endalias {
  std::vector<rumur::Ptr<rumur::AliasDecl>> decls;
  for (const std::tuple<std::string, rumur::Ptr<rumur::Expr>, rumur::location> &d : $2) {
    decls.push_back(rumur::Ptr<rumur::AliasDecl>::make(std::get<0>(d), std::get<1>(d), std::get<2>(d)));
  }
  $$ = rumur::Ptr<rumur::AliasRule>::make(decls, $4, @$);
};

begin_opt: BEGIN_TOK | %empty;

category: ASSERT {
  $$ = std::make_shared<rumur::Property::Category>(rumur::Property::ASSERTION);
} | ASSUME {
  $$ = std::make_shared<rumur::Property::Category>(rumur::Property::ASSUMPTION);
} | COVER {
  $$ = std::make_shared<rumur::Property::Category>(rumur::Property::COVER);
} | INVARIANT {
  $$ = std::make_shared<rumur::Property::Category>(rumur::Property::ASSERTION);
} | LIVENESS {
  $$ = std::make_shared<rumur::Property::Category>(rumur::Property::LIVENESS);
};

comma_opt: ',' | %empty;

decl: CONST exprdecls {
  for (const std::tuple<std::string, rumur::Ptr<rumur::Expr>, rumur::location> &d : $2) {
    $$.push_back(rumur::Ptr<rumur::ConstDecl>::make(std::get<0>(d), std::get<1>(d), std::get<2>(d)));
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
  $$ = rumur::Ptr<rumur::Field>::make($1, $3, @$);
} | designator '[' expr ']' {
  $$ = rumur::Ptr<rumur::Element>::make($1, $3, @$);
} | ID {
  $$ = rumur::Ptr<rumur::ExprID>::make($1, nullptr, @$);
};

else_opt: ELSE stmts {
  $$.push_back(rumur::IfClause(nullptr, $2, @$));
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
endswitch: END | ENDSWITCH;
endwhile: END | ENDWHILE;

expr: expr '?' expr ':' expr {
  $$ = rumur::Ptr<rumur::Ternary>::make($1, $3, $5, @$);
} | expr IMPLIES expr {
  $$ = rumur::Ptr<rumur::Implication>::make($1, $3, @$);
} | expr PIPEPIPE expr {
  $$ = rumur::Ptr<rumur::Or>::make($1, $3, @$);
} | expr LOR expr {
  $$ = rumur::Ptr<rumur::Or>::make($1, $3, @$);
} | expr AMPAMP expr {
  $$ = rumur::Ptr<rumur::And>::make($1, $3, @$);
} | expr LAND expr {
  $$ = rumur::Ptr<rumur::And>::make($1, $3, @$);
} | expr '|' expr {
  /* construct this as an ambiguous expression, that will later be resolved into
   * an Or or a Bor
   */
  $$ = rumur::Ptr<rumur::AmbiguousPipe>::make($1, $3, @$);
} | expr '^' expr {
  $$ = rumur::Ptr<rumur::Xor>::make($1, $3, @$);
} | expr '&' expr {
  /* construct this as an ambiguous expression, that will later be resolved into
   * an And or a Band
   */
  $$ = rumur::Ptr<rumur::AmbiguousAmp>::make($1, $3, @$);
} | '!' expr {
  $$ = rumur::Ptr<rumur::Not>::make($2, @$);
} | '~' expr {
  $$ = rumur::Ptr<rumur::Bnot>::make($2, @$);
} | expr '<' expr {
  $$ = rumur::Ptr<rumur::Lt>::make($1, $3, @$);
} | expr LEQ expr {
  $$ = rumur::Ptr<rumur::Leq>::make($1, $3, @$);
} | expr '>' expr {
  $$ = rumur::Ptr<rumur::Gt>::make($1, $3, @$);
} | expr GEQ expr {
  $$ = rumur::Ptr<rumur::Geq>::make($1, $3, @$);
} | expr DEQ expr {
  $$ = rumur::Ptr<rumur::Eq>::make($1, $3, @$);
} | expr '=' expr {
  $$ = rumur::Ptr<rumur::Eq>::make($1, $3, @$);
} | expr NEQ expr {
  $$ = rumur::Ptr<rumur::Neq>::make($1, $3, @$);
} | expr LSH expr {
  $$ = rumur::Ptr<rumur::Lsh>::make($1, $3, @$);
} | expr RSH expr {
  $$ = rumur::Ptr<rumur::Rsh>::make($1, $3, @$);
} | expr '+' expr {
  $$ = rumur::Ptr<rumur::Add>::make($1, $3, @$);
} | expr '-' expr {
  $$ = rumur::Ptr<rumur::Sub>::make($1, $3, @$);
} | '+' expr %prec '*' {
  $$ = $2;
  $$->loc = @$;
} | '-' expr %prec '*' {
  $$ = rumur::Ptr<rumur::Negative>::make($2, @$);
} | expr '*' expr {
  $$ = rumur::Ptr<rumur::Mul>::make($1, $3, @$);
} | expr '/' expr {
  $$ = rumur::Ptr<rumur::Div>::make($1, $3, @$);
} | expr '%' expr {
  $$ = rumur::Ptr<rumur::Mod>::make($1, $3, @$);
} | FORALL quantifier DO expr endforall {
    $$ = rumur::Ptr<rumur::Forall>::make(*$2, $4, @$);
} | EXISTS quantifier DO expr endexists {
    $$ = rumur::Ptr<rumur::Exists>::make(*$2, $4, @$);
} | designator {
  $$ = $1;
} | NUMBER {
  $$ = rumur::Ptr<rumur::Number>::make($1, @$);
} | '(' expr ')' {
  $$ = $2;
  $$->loc = @$;
} | ID '(' exprlist ')' {
  $$ = rumur::Ptr<rumur::FunctionCall>::make($1, $3, @$);
} | ISUNDEFINED '(' designator ')' {
  $$ = rumur::Ptr<rumur::IsUndefined>::make($3, @$);
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
    auto v = rumur::Ptr<rumur::VarDecl>::make(i.first, $4, @$);
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
  $$ = rumur::Ptr<rumur::Function>::make($2, $4, $6, $7, $9, @$);
};

property: category STRING expr {
  rumur::Property p(*$1, $3, @3);
  $$ = rumur::Ptr<rumur::PropertyRule>::make($2, p, @$);
} | category expr string_opt {
  rumur::Property p(*$1, $2, @2);
  $$ = rumur::Ptr<rumur::PropertyRule>::make($3, p, @$);
};

quantifier: ID ':' typeexpr {
  $$ = std::make_shared<rumur::Quantifier>($1, $3, @$);
} | ID COLON_EQ expr TO expr BY expr {
  $$ = std::make_shared<rumur::Quantifier>($1, $3, $5, $7, @$);
} | ID COLON_EQ expr TO expr {
  $$ = std::make_shared<rumur::Quantifier>($1, $3, $5, @$);
};

quantifiers: quantifiers semis quantifier {
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
  $$ = rumur::Ptr<rumur::Ruleset>::make($2, $4, @$);
};

semi_opt: semi_opt ';' | %empty;

semis: semi_opt ';';

simplerule: RULE string_opt guard_opt decls_header stmts endrule {
  $$ = rumur::Ptr<rumur::SimpleRule>::make($2, $3, $4, $5, @$);
};

startstate: STARTSTATE string_opt decls_header stmts endstartstate {
  $$ = rumur::Ptr<rumur::StartState>::make($2, $3, $4, @$);
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
  std::vector<rumur::Ptr<rumur::AliasDecl>> decls;
  for (const std::tuple<std::string, rumur::Ptr<rumur::Expr>, rumur::location> &d : $2) {
    decls.push_back(rumur::Ptr<rumur::AliasDecl>::make(std::get<0>(d), std::get<1>(d), std::get<2>(d)));
  }
  $$ = rumur::Ptr<rumur::AliasStmt>::make(decls, $4, @$);
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
  $$ = rumur::Ptr<rumur::If>::make(cs, @$);
} | PUT STRING {
  $$ = rumur::Ptr<rumur::Put>::make($2, @$);
} | PUT expr {
  $$ = rumur::Ptr<rumur::Put>::make($2, @$);
} | RETURN {
  $$ = rumur::Ptr<rumur::Return>::make(nullptr, @$);
} | RETURN expr {
  $$ = rumur::Ptr<rumur::Return>::make($2, @$);
} | UNDEFINE designator {
  $$ = rumur::Ptr<rumur::Undefine>::make($2, @$);
} | ID '(' exprlist ')' {
  $$ = rumur::Ptr<rumur::ProcedureCall>::make($1, $3, @$);
} | WHILE expr DO stmts endwhile {
  $$ = rumur::Ptr<rumur::While>::make($2, $4, @$);
} | SWITCH expr switchcases endswitch {
  $$ = rumur::Ptr<rumur::Switch>::make($2, $3, @$);
};

stmts: stmts_cont stmt semi_opt {
  $$ = $1;
  $$.push_back($2);
} | stmt semi_opt {
  $$.push_back($1);
} | %empty {
};

stmts_cont: stmts_cont stmt semis {
  $$ = $1;
  $$.push_back($2);
} | stmt semis {
  $$.push_back($1);
};

string_opt: STRING {
  $$ = $1;
} | %empty {
  /* nothing required */
};

switchcases: switchcases_cont ELSE stmts {
  $$ = $1;
  $$.push_back(rumur::SwitchCase(std::vector<Ptr<rumur::Expr>>(), $3, @$));
} | switchcases_cont {
  $$ = $1;
};

switchcases_cont: switchcases_cont CASE exprlist ':' stmts {
  $$ = $1;
  $$.push_back(rumur::SwitchCase($3, $5, @$));
} | %empty {
  /* nothing required */
};

typedecl: id_list_opt ':' typeexpr {
  for (const std::pair<std::string, rumur::location> &m : $1) {
    $$.push_back(rumur::Ptr<rumur::TypeDecl>::make(m.first, $3, @$));
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
  $$ = rumur::Ptr<rumur::TypeExprID>::make("boolean", nullptr, @$);
} | ID {
  $$ = rumur::Ptr<rumur::TypeExprID>::make($1, nullptr, @$);
} | expr DOTDOT expr {
  $$ = rumur::Ptr<rumur::Range>::make($1, $3, @$);
} | ENUM '{' id_list_opt '}' {
  $$ = rumur::Ptr<rumur::Enum>::make($3, @$);
} | RECORD vardecls endrecord {
  $$ = rumur::Ptr<rumur::Record>::make($2, @$);
} | ARRAY '[' typeexpr ']' OF typeexpr {
  $$ = rumur::Ptr<rumur::Array>::make($3, $6, @$);
} | SCALARSET '(' expr ')' {
  $$ = rumur::Ptr<rumur::Scalarset>::make($3, @$);
};

vardecl: id_list_opt ':' typeexpr {
  for (const std::pair<std::string, rumur::location> &m : $1) {
    $$.push_back(rumur::Ptr<rumur::VarDecl>::make(m.first, $3, @$));
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
