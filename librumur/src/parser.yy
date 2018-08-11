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
  #include <utility>
  #include <vector>

  /* Redirect yylex to call our derived scanner. */
  #ifdef yylex
    #undef yylex
  #endif
  #define yylex sc.yylex

  /* Current offset into model state. Used when declaring variables. */
  static size_t offset;

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
%parse-param { rumur::Model *&output }

  /* And also a symbol table we'll use for relating identifiers back to the
   * target they refer to.
   */
%parse-param { rumur::Symtab &symtab }

%token ARRAY
%token ARROW
%token ASSERT
%token ASSUME
%token BEGIN_TOK
%token BY
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
%token ENDIF
%token ENDRECORD
%token ENDRULE
%token ENDRULESET
%token ENDSTARTSTATE
%token ENUM
%token ERROR
%token EXISTS
%token FOR
%token FORALL
%token GEQ
%token <std::string> ID
%token IF
%token IMPLIES
%token INVARIANT
%token LEQ
%token NEQ
%token <std::string> NUMBER
%token OF
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
%type <rumur::Decl*>                                         constdecl
%type <std::vector<rumur::Decl*>>                            constdecls
%type <std::vector<rumur::Decl*>>                            decl
%type <std::vector<rumur::Decl*>>                            decls
%type <std::vector<rumur::Decl*>>                            decls_header
%type <rumur::Lvalue*>                                       designator
%type <std::vector<rumur::IfClause>>                         else_opt
%type <std::vector<rumur::IfClause>>                         elsifs
%type <rumur::Expr*>                                         expr
%type <rumur::Expr*>                                         guard_opt
%type <std::vector<std::pair<std::string, rumur::location>>> id_list
%type <std::vector<std::pair<std::string, rumur::location>>> id_list_opt
%type <rumur::PropertyRule*>                                 property
%type <rumur::Quantifier*>                                   quantifier
%type <std::vector<rumur::Quantifier*>>                      quantifiers
%type <rumur::Rule*>                                         rule
%type <rumur::Ruleset*>                                      ruleset
%type <std::vector<rumur::Rule*>>                            rules
%type <std::vector<rumur::Rule*>>                            rules_cont
%type <rumur::SimpleRule*>                                   simplerule
%type <rumur::StartState*>                                   startstate
%type <rumur::Stmt*>                                         stmt
%type <std::vector<rumur::Stmt*>>                            stmts
%type <std::vector<rumur::Stmt*>>                            stmts_cont
%type <std::string>                                          string_opt
%type <rumur::Decl*>                                         typedecl
%type <std::vector<rumur::Decl*>>                            typedecls
%type <rumur::TypeExpr*>                                     typeexpr
%type <std::vector<rumur::VarDecl*>>                         vardecl
%type <std::vector<rumur::VarDecl*>>                         vardecls

%%

model: {
      /* Reset offset, in case we were previously parsing a model. */
      offset = 0;
    } decls rules {
  output = new rumur::Model(std::move($2), std::move($3), @$);
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
  for (rumur::VarDecl *d : $2) {
    // Account for whether this declaration is part of the state
    if (symtab.is_global_scope()) {
      d->state_variable = true;
      d->offset = offset;
      offset += d->type->width();
    }
    symtab.declare(d->name, *d);
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
  $$ = new rumur::ConstDecl($1, $3, @$);
  symtab.declare($1, *$$);
};

typedecls: typedecls typedecl {
  $$ = $1;
  $$.push_back($2);
} | %empty {
  /* nothing required */
};

typedecl: ID ':' typeexpr ';' {
  $$ = new rumur::TypeDecl($1, $3, @$);
  symtab.declare($1, *$3);
};

typeexpr: ID {
  const rumur::TypeExpr *t = symtab.lookup<rumur::TypeExpr>($1, @$);
  if (t == nullptr) {
    throw rumur::Error("unknown type ID \"" + $1 + "\"", @1);
  }
  $$ = new rumur::TypeExprID($1, t->clone(), @$);
} | expr DOTDOT expr {
  $$ = new rumur::Range($1, $3, @$);
} | ENUM '{' id_list_opt '}' {
  auto e = new rumur::Enum(std::move($3), @$);
  /* Register all the enum members so they can be referenced later. */
  const rumur::TypeDecl td("", new rumur::Enum(*e), @$);
  for (const std::pair<std::string, rumur::location> &m : e->members) {
    symtab.declare(m.first, td);
  }
  $$ = e;
} | RECORD vardecls endrecord {
  $$ = new rumur::Record(std::move($2), @$);
} | ARRAY '[' typeexpr ']' OF typeexpr {
  $$ = new rumur::Array($3, $6, @$);
} | SCALARSET '(' expr ')' {
  $$ = new rumur::Scalarset($3, @$);
};

vardecls: vardecls vardecl {
  $$ = $1;
  std::move($2.begin(), $2.end(), std::back_inserter($$));
} | %empty {
  /* nothing required */
};

vardecl: id_list_opt ':' typeexpr ';' {
  for (const std::pair<std::string, rumur::location> &m : $1) {
    $$.push_back(new rumur::VarDecl(m.first, $3, m.second));
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
} | simplerule {
  $$ = $1;
} | property {
  $$ = $1;
} | ruleset {
  $$ = $1;
};

startstate: STARTSTATE string_opt { symtab.open_scope(); } decls_header stmts { symtab.close_scope(); } endstartstate {
  $$ = new rumur::StartState($2, std::move($4), std::move($5), @$);
};

simplerule: RULE string_opt guard_opt { symtab.open_scope(); } decls_header stmts { symtab.close_scope(); } endrule {
  $$ = new rumur::SimpleRule($2, $3, std::move($5), std::move($6), @$);
};

property: category STRING expr {
  rumur::Property p($1, $3, @3);
  $$ = new rumur::PropertyRule($2, p, @$);
} | category expr string_opt {
  rumur::Property p($1, $2, @2);
  $$ = new rumur::PropertyRule($3, p, @$);
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
  $$ = new rumur::Ruleset(std::move($3), std::move($5), @$);
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
  $$ = new rumur::PropertyStmt(p, $2, @$);
} | category expr string_opt {
  rumur::Property p($1, $2, @2);
  $$ = new rumur::PropertyStmt(p, $3, @$);
} | designator COLON_EQ expr {
  $$ = new rumur::Assignment($1, $3, @$);
} | ERROR STRING {
  $$ = new rumur::ErrorStmt($2, @$);
} | FOR quantifier {
    symtab.open_scope();
    symtab.declare($2->var->name, *$2->var);
  } DO stmts endfor {
  $$ = new rumur::For($2, std::move($5), @$);
  symtab.close_scope();
} | IF expr THEN stmts elsifs else_opt endif {
  std::vector<rumur::IfClause> cs = {
    rumur::IfClause($2, std::move($4), rumur::location(@1.begin, @4.end)) };
  cs.insert(cs.end(), $5.begin(), $5.end());
  cs.insert(cs.end(), $6.begin(), $6.end());
  $$ = new rumur::If(std::move(cs), @$);
} | RETURN {
  $$ = new rumur::Return(nullptr, @$);
} | RETURN expr {
  $$ = new rumur::Return($2, @$);
} | UNDEFINE designator {
  $$ = new rumur::Undefine($2, @$);
};

elsifs: elsifs ELSIF expr THEN stmts {
  $$ = $1;
  $$.push_back(rumur::IfClause($3, std::move($5), rumur::location(@2.begin, @5.end)));
} | %empty {
};

else_opt: ELSE stmts {
  $$.push_back(rumur::IfClause(nullptr, std::move($2), @$));
} | %empty {
};

endif: END | ENDIF;

endstartstate: END | ENDSTARTSTATE;

string_opt: STRING {
  $$ = $1;
} | %empty {
  /* nothing required */
};

semi_opt: ';' | %empty;

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
} | expr DEQ expr {
  $$ = new rumur::Eq($1, $3, @$);
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
} | FORALL quantifier {
    symtab.open_scope();
    symtab.declare($2->var->name, *$2->var);
  } DO expr endforall {
    $$ = new rumur::Forall($2, $5, @$);
    symtab.close_scope();
} | EXISTS quantifier {
    symtab.open_scope();
    symtab.declare($2->var->name, *$2->var);
  } DO expr endexists {
    $$ = new rumur::Exists($2, $5, @$);
    symtab.close_scope();
} | designator {
  $$ = $1;
} | NUMBER {
  $$ = new rumur::Number($1, @$);
} | '(' expr ')' {
  $$ = $2;
  $$->loc = @$;
};

quantifier: ID ':' typeexpr {
  $$ = new rumur::Quantifier($1, $3, @$);
} | ID ':' expr TO expr BY expr {
  $$ = new rumur::Quantifier($1, $3, $5, $7, @$);
} | ID ':' expr TO expr {
  $$ = new rumur::Quantifier($1, $3, $5, @$);
};

quantifiers: quantifiers ';' quantifier {
  $$ = $1;
  symtab.declare($3->var->name, *$3->var);
  $$.push_back($3);
} | quantifier {
  symtab.declare($1->var->name, *$1->var);
  $$.push_back($1);
};

designator: designator '.' ID {
  $$ = new rumur::Field($1, $3, @$);
} | designator '[' expr ']' {
  $$ = new rumur::Element($1, $3, @$);
} | ID {
  auto d = symtab.lookup<rumur::Decl>($1, @$);
  assert(d != nullptr);
  $$ = new rumur::ExprID($1, d, @$);
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
