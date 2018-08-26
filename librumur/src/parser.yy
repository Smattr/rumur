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

  #include <memory>
  #include <rumur/Decl.h>
  #include <rumur/Expr.h>
  #include <rumur/Function.h>
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
  #include <gmpxx.h>
  #include <iostream>
  #include <iterator>
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
%parse-param { std::shared_ptr<rumur::Model> &output }

  /* And also a symbol table we'll use for relating identifiers back to the
   * target they refer to.
   */
%parse-param { rumur::Symtab &symtab }

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

%type <rumur::Property::Category>                            category
%type <std::shared_ptr<rumur::Decl>>                         constdecl
%type <std::vector<std::shared_ptr<rumur::Decl>>>            constdecls
%type <std::vector<std::shared_ptr<rumur::Decl>>>            decl
%type <std::vector<std::shared_ptr<rumur::Decl>>>            decls
%type <std::vector<std::shared_ptr<rumur::Decl>>>            decls_header
%type <std::shared_ptr<rumur::Lvalue>>                       designator
%type <std::vector<rumur::IfClause>>                         else_opt
%type <std::vector<rumur::IfClause>>                         elsifs
%type <std::shared_ptr<rumur::Expr>>                         expr
%type <std::vector<std::shared_ptr<rumur::Expr>>>            exprlist
%type <std::vector<std::shared_ptr<rumur::Expr>>>            exprlist_cont
%type <std::shared_ptr<rumur::Expr>>                         guard_opt
%type <std::vector<std::pair<std::string, rumur::location>>> id_list
%type <std::vector<std::pair<std::string, rumur::location>>> id_list_opt
%type <std::vector<std::shared_ptr<rumur::Parameter>>>       parameter
%type <std::vector<std::shared_ptr<rumur::Parameter>>>       parameters
%type <std::vector<std::shared_ptr<rumur::Parameter>>>       parameters_cont
%type <std::shared_ptr<rumur::Function>>                     procdecl
%type <std::vector<std::shared_ptr<rumur::Function>>>        procdecls
%type <std::shared_ptr<rumur::PropertyRule>>                 property
%type <std::shared_ptr<rumur::Quantifier>>                   quantifier
%type <std::vector<std::shared_ptr<rumur::Quantifier>>>      quantifiers
%type <std::shared_ptr<rumur::TypeExpr>>                     return_type
%type <std::shared_ptr<rumur::Rule>>                         rule
%type <std::shared_ptr<rumur::Ruleset>>                      ruleset
%type <std::vector<std::shared_ptr<rumur::Rule>>>            rules
%type <std::vector<std::shared_ptr<rumur::Rule>>>            rules_cont
%type <std::shared_ptr<rumur::SimpleRule>>                   simplerule
%type <std::shared_ptr<rumur::StartState>>                   startstate
%type <std::shared_ptr<rumur::Stmt>>                         stmt
%type <std::vector<std::shared_ptr<rumur::Stmt>>>            stmts
%type <std::vector<std::shared_ptr<rumur::Stmt>>>            stmts_cont
%type <std::string>                                          string_opt
%type <std::shared_ptr<rumur::Decl>>                         typedecl
%type <std::vector<std::shared_ptr<rumur::Decl>>>            typedecls
%type <std::shared_ptr<rumur::TypeExpr>>                     typeexpr
%type <std::vector<std::shared_ptr<rumur::VarDecl>>>         vardecl
%type <std::vector<std::shared_ptr<rumur::VarDecl>>>         vardecls
%type <bool>                                                 var_opt

%%

model: decls procdecls rules {
  output = std::make_shared<rumur::Model>(std::move($1), std::move($2), std::move($3), @$);
};

decls: decls decl {
  $$ = $1;
  std::move($2.begin(), $2.end(), std::back_inserter($$));
} | %empty {
  /* nothing required */
};

decl: CONST constdecls {
  $$ = $2;
} | TYPE typedecls {
  $$ = $2;
} | VAR vardecls {
  for (std::shared_ptr<rumur::VarDecl> &d : $2) {
    symtab.declare(d->name, d);
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
  $$ = std::make_shared<rumur::ConstDecl>($1, $3, @$);
  symtab.declare($1, $$);
};

typedecls: typedecls typedecl {
  $$ = $1;
  $$.push_back($2);
} | %empty {
  /* nothing required */
};

typedecl: ID ':' typeexpr ';' {
  $$ = std::make_shared<rumur::TypeDecl>($1, $3, @$);
  symtab.declare($1, $3);
};

typeexpr: BOOLEAN {
  std::shared_ptr<rumur::TypeExpr> t = symtab.lookup<rumur::TypeExpr>("boolean", @$);
  assert(t != nullptr && "boolean built-in type unknown");
  $$ = std::make_shared<rumur::TypeExprID>("boolean", t, @$);
} | ID {
  std::shared_ptr<rumur::TypeExpr> t = symtab.lookup<rumur::TypeExpr>($1, @$);
  if (t == nullptr) {
    throw rumur::Error("unknown type ID \"" + $1 + "\"", @1);
  }
  $$ = std::make_shared<rumur::TypeExprID>($1, t, @$);
} | expr DOTDOT expr {
  $$ = std::make_shared<rumur::Range>($1, $3, @$);
} | ENUM '{' id_list_opt '}' {
  auto e = std::make_shared<rumur::Enum>(std::move($3), @$);
  /* Register all the enum members so they can be referenced later. */
  mpz_class index = 0;
  for (const std::pair<std::string, rumur::location> &m : e->members) {
    auto cd = std::make_shared<rumur::ConstDecl>(m.first,
      std::make_shared<rumur::Number>(index, m.second), e, m.second);
    symtab.declare(m.first, cd);
    index++;
  }
  $$ = e;
} | RECORD vardecls endrecord {
  $$ = std::make_shared<rumur::Record>(std::move($2), @$);
} | ARRAY '[' typeexpr ']' OF typeexpr {
  $$ = std::make_shared<rumur::Array>($3, $6, @$);
} | SCALARSET '(' expr ')' {
  $$ = std::make_shared<rumur::Scalarset>($3, @$);
};

vardecls: vardecls vardecl {
  $$ = $1;
  std::move($2.begin(), $2.end(), std::back_inserter($$));
} | %empty {
  /* nothing required */
};

vardecl: id_list_opt ':' typeexpr ';' {
  for (const std::pair<std::string, rumur::location> &m : $1) {
    $$.push_back(std::make_shared<rumur::VarDecl>(m.first, $3, m.second));
  }
};

endrecord: END | ENDRECORD;

begin_opt: BEGIN_TOK | %empty;

endfunction: END | ENDFUNCTION | ENDPROCEDURE;

function: FUNCTION | PROCEDURE;

parameter: var_opt id_list ':' typeexpr {
  for (const std::pair<std::string, rumur::location> &i : $2) {
    auto v = std::make_shared<rumur::VarDecl>(i.first, std::shared_ptr<rumur::TypeExpr>($4->clone()), i.second);
    symtab.declare(v->name, v);
    $$.push_back(std::make_shared<rumur::Parameter>(v, $1, @$));
  }
};

parameters: parameters_cont parameter {
  $$ = $1;
  std::move($2.begin(), $2.end(), std::back_inserter($$));
} | parameter {
  $$ = $1;
} | %empty {
};

parameters_cont: parameters_cont parameter ';' {
  $$ = $1;
  std::move($2.begin(), $2.end(), std::back_inserter($$));
} | parameter ';' {
  $$ = $1;
};

procdecl: function ID { symtab.open_scope(); } '(' parameters ')' return_type decls begin_opt stmts { symtab.close_scope(); } endfunction ';' {
  $$ = std::make_shared<rumur::Function>($2, std::move($5), $7, std::move($8), std::move($10), @$);
  symtab.declare($$->name, $$);
};

procdecls: procdecls procdecl {
  $$ = $1;
  $$.push_back($2);
} | %empty {
};

return_type: ':' typeexpr semi_opt {
  $$ = $2;
} | semi_opt {
  $$ = nullptr;
};

var_opt: VAR {
  $$ = true;
} | %empty {
  $$ = false;
};

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
} | simplerule {
  $$ = $1;
} | property {
  $$ = $1;
} | ruleset {
  $$ = $1;
};

startstate: STARTSTATE string_opt { symtab.open_scope(); } decls_header stmts { symtab.close_scope(); } endstartstate {
  $$ = std::make_shared<rumur::StartState>($2, std::move($4), std::move($5), @$);
};

simplerule: RULE string_opt guard_opt { symtab.open_scope(); } decls_header stmts { symtab.close_scope(); } endrule {
  $$ = std::make_shared<rumur::SimpleRule>($2, $3, std::move($5), std::move($6), @$);
};

property: category STRING expr {
  rumur::Property p($1, $3, @3);
  $$ = std::make_shared<rumur::PropertyRule>($2, p, @$);
} | category expr string_opt {
  rumur::Property p($1, $2, @2);
  $$ = std::make_shared<rumur::PropertyRule>($3, p, @$);
};

category: ASSERT {
  $$ = rumur::Property::ASSERTION;
} | ASSUME {
  $$ = rumur::Property::ASSUMPTION;
} | INVARIANT {
  $$ = rumur::Property::ASSERTION;
} | PROPERTY {
  $$ = rumur::Property::DISABLED;
};

ruleset: RULESET { symtab.open_scope(); } quantifiers DO rules { symtab.close_scope(); } endruleset {
  $$ = std::make_shared<rumur::Ruleset>(std::move($3), std::move($5), @$);
};

guard_opt: expr ARROW {
  $$ = $1;
} | %empty {
  $$ = nullptr;
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

stmt: category STRING expr {
  rumur::Property p($1, $3, @3);
  $$ = std::make_shared<rumur::PropertyStmt>(p, $2, @$);
} | category expr string_opt {
  rumur::Property p($1, $2, @2);
  $$ = std::make_shared<rumur::PropertyStmt>(p, $3, @$);
} | designator COLON_EQ expr {
  $$ = std::make_shared<rumur::Assignment>($1, $3, @$);
} | ERROR STRING {
  $$ = std::make_shared<rumur::ErrorStmt>($2, @$);
} | CLEAR designator {
  $$ = std::make_shared<rumur::Clear>($2, @$);
} | FOR quantifier {
    symtab.open_scope();
    symtab.declare($2->var->name, $2->var);
  } DO stmts endfor {
  $$ = std::make_shared<rumur::For>($2, std::move($5), @$);
  symtab.close_scope();
} | IF expr THEN stmts elsifs else_opt endif {
  std::vector<rumur::IfClause> cs = {
    rumur::IfClause($2, std::move($4), rumur::location(@1.begin, @4.end)) };
  cs.insert(cs.end(), $5.begin(), $5.end());
  cs.insert(cs.end(), $6.begin(), $6.end());
  $$ = std::make_shared<rumur::If>(std::move(cs), @$);
} | RETURN {
  $$ = std::make_shared<rumur::Return>(std::shared_ptr<Expr>(), @$);
} | RETURN expr {
  $$ = std::make_shared<rumur::Return>($2, @$);
} | UNDEFINE designator {
  $$ = std::make_shared<rumur::Undefine>($2, @$);
} | ID '(' exprlist ')' {
  std::shared_ptr<rumur::Function> f = symtab.lookup<rumur::Function>($1, @$);
  assert(f != nullptr);
  $$ = std::make_shared<rumur::ProcedureCall>($1, f, std::move($3), @$);
};

elsifs: elsifs ELSIF expr THEN stmts {
  $$ = $1;
  $$.push_back(rumur::IfClause($3, std::move($5), rumur::location(@2.begin, @5.end)));
} | %empty {
};

else_opt: ELSE stmts {
  $$.push_back(rumur::IfClause(std::shared_ptr<Expr>(), std::move($2), @$));
} | %empty {
};

endif: END | ENDIF;

endstartstate: END | ENDSTARTSTATE;

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

string_opt: STRING {
  $$ = $1;
} | %empty {
  /* nothing required */
};

semi_opt: ';' | %empty;

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
} | FORALL quantifier {
    symtab.open_scope();
    symtab.declare($2->var->name, $2->var);
  } DO expr endforall {
    $$ = std::make_shared<rumur::Forall>($2, $5, @$);
    symtab.close_scope();
} | EXISTS quantifier {
    symtab.open_scope();
    symtab.declare($2->var->name, $2->var);
  } DO expr endexists {
    $$ = std::make_shared<rumur::Exists>($2, $5, @$);
    symtab.close_scope();
} | designator {
  $$ = $1;
} | NUMBER {
  $$ = std::make_shared<rumur::Number>($1, @$);
} | '(' expr ')' {
  $$ = $2;
  $$->loc = @$;
} | ID '(' exprlist ')' {
  std::shared_ptr<rumur::Function> f = symtab.lookup<rumur::Function>($1, @$);
  assert(f != nullptr);
  $$ = std::make_shared<rumur::FunctionCall>(f, std::move($3), @$);
};

quantifier: ID ':' typeexpr {
  $$ = std::make_shared<rumur::Quantifier>($1, $3, @$);
} | ID ':' expr TO expr BY expr {
  $$ = std::make_shared<rumur::Quantifier>($1, $3, $5, $7, @$);
} | ID ':' expr TO expr {
  $$ = std::make_shared<rumur::Quantifier>($1, $3, $5, @$);
};

quantifiers: quantifiers ';' quantifier {
  $$ = $1;
  symtab.declare($3->var->name, $3->var);
  $$.push_back($3);
} | quantifier {
  symtab.declare($1->var->name, $1->var);
  $$.push_back($1);
};

designator: designator '.' ID {
  $$ = std::make_shared<rumur::Field>($1, $3, @$);
} | designator '[' expr ']' {
  $$ = std::make_shared<rumur::Element>($1, $3, @$);
} | ID {
  std::shared_ptr<rumur::Decl> d = symtab.lookup<rumur::Decl>($1, @$);
  assert(d != nullptr);
  $$ = std::make_shared<rumur::ExprID>($1, d, @$);
};

endfor: END | ENDFOR;

endforall: END | ENDFORALL;

endexists: END | ENDEXISTS;

endrule: END | ENDRULE;

endruleset: END | ENDRULESET;

  /* Support optional trailing comma to make it easier for tools that generate
   * an input mdoels.
   */
id_list_opt: id_list comma_opt {
  $$ = $1;
} | %empty {
};

id_list: id_list ',' ID {
  $$ = $1;
  $$.emplace_back(std::make_pair($3, @3));
} | ID {
  $$.emplace_back(std::make_pair($1, @$));
};

comma_opt: ',' | %empty;

%%

void rumur::parser::error(const location_type &loc, const std::string &message) {
  throw Error(message, loc);
}
