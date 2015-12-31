// This is intended to be a faithful translation into PlyPlus's EBNF from
// https://www.cs.ubc.ca/~ajh/courses/cpsc513/assign-token/User.Manual

start: program ;

ID: '[a-zA-Z][a-zA-Z0-9_]*'
        (%unless
            ALIAS: 'alias';
            ARRAY: 'array';
            ASSERT: 'assert';
            BEGIN: 'begin';
            BY: 'by';
            CASE: 'case';
            CLEAR: 'clear';
            CONST: 'const';
            DO: 'do';
            ELSE: 'else';
            ELSIF: 'elsif';
            END: 'end';
            ENDALIAS: 'endalias';
            ENDEXISTS: 'endexists';
            ENDFOR: 'endfor';
            ENDFORALL: 'endforall';
            ENDFUNCTION: 'endfunction';
            ENDIF: 'endif';
            ENDPROCEDURE: 'endprocedure';
            ENDRECORD: 'endrecord';
            ENDRULE: 'endrule';
            ENDRULESET: 'endruleset';
            ENDSTARTSTATE: 'endstartstate';
            ENDSWITCH: 'endswitch';
            ENDWHILE: 'endwhile';
            ENUM: 'enum';
            ERROR: 'error';
            EXISTS: 'exists';
            FOR: 'for';
            FORALL: 'forall';
            FUNCTION: 'function';
            IF: 'if';
            INVARIANT: 'invariant';
            OF: 'of';
            PUT: 'put';
            PROCEDURE: 'procedure';
            RECORD: 'record';
            RETURN: 'return';
            RULE: 'rule';
            RULESET: 'ruleset';
            STARTSTATE: 'startstate';
            SWITCH: 'switch';
            THEN: 'then';
            TO: 'to';
            TYPE: 'type';
            VAR: 'var';
            WHILE: 'while';
            );

string: '"[^"]*"'; // "

integer_constant: '\d+';

// comment: ('--[^\n]*\n' | '/\*.*\*/');

program: decl* procdecl* rules*;

@decl: CONST (constdecl ';')*
     | TYPE (typedecl ';')*
     | VAR (vardecl ';')*
     ;

constdecl: symbol ':' expr;

typedecl: symbol ':' typeexpr;

typeexpr: symbol
        | expr '\.\.' expr
        | enum '\{' symbol (',' symbol)* '\}'
        | record vardecl* endrecord
        | array '\[' typeexpr '\]' OF typeexpr
        ;

vardecl: symbol (',' symbol)* ':' typeexpr;

@procdecl: procedure
         | function
         ;

procedure: PROCEDURE symbol '\(' (formal (';' formal)* )? '\)' ';' (decl* BEGIN)? stmts? endprocedure ';';

function: FUNCTION symbol '\(' (formal (';' formal)* )? '\)' ':' typeexpr ';' (decl* BEGIN)? stmts? endfunction ';';

formal: var? symbol (',' symbol)* ':' typeexpr;

designator: symbol ('\.' symbol | '\[' expr '\]')*;

expr: expr question expr colon expr
    | expr1
    ;
expr1: expr1 implies expr1
      | expr2
      ;
expr2: expr2 or expr2
      | expr3
      ;
expr3: expr3 and expr3
      | expr4
      ;
expr4: not expr4
      | expr5
      ;
expr5: expr5 lt expr5
      | expr5 lte expr5
      | expr5 eq expr5
      | expr5 neq expr5
      | expr5 gt expr5
      | expr5 gte expr5
      | expr6
      ;
expr6: expr6 add expr6
      | expr6 sub expr6
      | expr7
      ;
expr7: expr7 mul expr7
      | expr7 div expr7
      | expr7 mod expr7
      | expr8
      ;
/* XXX: The docs don't give a precedence for other operators, so just assume
 * they're all the lowest precedence.
 */
expr8: lbrace expr rbrace
      | designator
      | integer_constant
      | symbol lbrace actuals rbrace
      | forall quantifier DO expr endforall
      | exists quantifier DO expr endexists
      ;

stmts: stmt (';' stmt?)*;

@stmt: assignment
     | ifstmt
     | switchstmt
     | forstmt
     | whilestmt
     | aliasstmt
     | proccall
     | clearstmt
     | errorstmt
     | assertstmt
     | putstmt
     | returnstmt
     ;

assignment: designator ':=' expr;

ifstmt: IF expr THEN stmts?
             (elsif expr THEN stmts?)*
             (else stmts?)?
        endif;

elsif: ELSIF;
else: ELSE;

switchstmt: SWITCH expr
               (case expr (',' expr)* ':' stmts?)*
               (else stmts?)?
            endswitch;

case: CASE;

forstmt: FOR quantifier DO stmts? endfor;

quantifier: symbol ':' typeexpr
          | symbol ':=' expr TO expr (BY expr)?
          ;

whilestmt: WHILE expr DO stmts? endwhile;

aliasstmt: ALIAS alias (';' alias)* DO stmts? endalias;

alias: symbol ':' expr;

proccall: symbol '\(' expr (',' expr)* '\)';

clearstmt: CLEAR designator;

errorstmt: ERROR string;

assertstmt: ASSERT expr string?;

putstmt: PUT (expr | string);

returnstmt: RETURN expr?;

// No description is given of this, so I'm just going to decide it can be any
// expression.
actuals:
       | expr (',' expr)*
       ;

@rules: rule (';' rule)* ';'?;

@rule: simplerule
     | startstate
     | invariant
     | ruleset
     | aliasrule
     ;

simplerule: RULE string?
               (expr '==>')?
               (decl* BEGIN)?
               stmts?
            endrule;

startstate: STARTSTATE string?
               (decl* BEGIN)?
               stmts?
            endstartstate;

invariant: INVARIANT string? expr;

ruleset: RULESET quantifier
         (';' quantifier)* DO rules? endruleset;

aliasrule: ALIAS alias (';' alias)* DO rules? endalias;

symbol: ID;

// Irrelevant end-synonym handling

endrecord: ENDRECORD
         | END
         ;

endprocedure: ENDPROCEDURE
            | END
            ;

endfunction: ENDFUNCTION
           | END
           ;

endforall: ENDFORALL
         | END
         ;

endexists: ENDEXISTS
         | END
         ;

endif: ENDIF
     | END
     ;

endswitch: ENDSWITCH
         | END
         ;

endfor: ENDFOR
      | END
      ;

endwhile: ENDWHILE
        | END
        ;

endalias: ENDALIAS
        | END
        ;

endrule: ENDRULE
       | END
       ;

endstartstate: ENDSTARTSTATE
             | END
             ;

endruleset: ENDRULESET
          | END
          ;

enum: ENUM;

record: RECORD;

array: ARRAY;

var: VAR;

WHITESPACE: '[ \t\n]+' (%ignore) ;

lbrace: '\(';
rbrace: '\)';
forall: FORALL;
exists: EXISTS;
add: '\+';
sub: '-';
mul: '\*';
div: '/';
mod: '%';
not: '!';
or: '\|';
and: '&';
implies: '->';
lt: '<';
lte: '<=';
gt: '>';
gte: '>=';
eq: '=';
neq: '!=';
question: '\?';
colon: ':';
