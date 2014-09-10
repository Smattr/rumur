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

decl: CONST (constdecl ';')*
    | TYPE (typedecl ';')*
    | VAR (vardecl ';')*
    ;

constdecl: symbol ':' expr;

typedecl: symbol ':' typeexpr;

typeexpr: symbol
        | expr '\.\.' expr
        | ENUM '\{' symbol (',' symbol)* '\}'
        | RECORD vardecl* endrecord
        | ARRAY '\[' typeexpr '\]' OF typeexpr
        ;

vardecl: symbol (',' symbol)* ':' typeexpr;

procdecl: procedure
        | function
        ;

procedure: PROCEDURE symbol '\(' (formal (';' formal)* )? '\)' ';' (decl* BEGIN)? stmts? endprocedure ';';

function: FUNCTION symbol '\(' (formal (';' formal)* )? '\)' ':' typeexpr ';' (decl* BEGIN)? stmts? endfunction ';';

formal: VAR? symbol (',' symbol)* ':' typeexpr;

designator: symbol ('\.' symbol | '\[' expr '\]')*;

expr: lbrace expr rbrace
    | designator
    | integer_constant
    | symbol lbrace actuals rbrace
    | FORALL quantifier DO expr endforall
    | EXISTS quantifier DO expr endexists
    | expr add expr
    | expr sub expr
    | expr mul expr
    | expr div expr
    | expr mod expr
    | not expr
    | expr or expr
    | expr and expr
    | expr implies expr
    | expr lt expr
    | expr lte expr
    | expr gt expr
    | expr gte expr
    | expr eq expr
    | expr neq expr
    | expr question expr colon expr
    ;

stmts: stmt (';' stmt?)*;

stmt: assignment
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
             (ELSIF expr THEN stmts?)*
             (ELSE stmts?)?
        endif;

switchstmt: SWITCH expr
               (CASE expr (',' expr)* ':' stmts?)*
               (ELSE stmts?)?
            endswitch;

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

rules: rule (';' rule)* ';'?;

rule: simplerule
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

WHITESPACE: '[ \t\n]+' (%ignore) ;

lbrace: '\(';
rbrace: '\)';
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
