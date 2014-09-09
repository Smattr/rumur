'''
An encoding of the Murphi grammar rules into PLY.
https://www.cs.ubc.ca/~ajh/courses/cpsc513/assign-token/User.Manual
'''

import Tokens

def p_program(t):
    '''program : decls procdecls rules'''
    t[0] = Program(t[1], t[2], t[3])

def p_decls(t):
    '''decls :
             | decl decls'''
    if len(t) == 1:
        t[0] = []
    else:
        t[0] = [t[1]] + t[2]

def p_procdecls(t):
    '''procdecls :
                 | procdecl procdecls'''
    if len(t) == 1:
        t[0] = []
    else:
        t[0] = [t[1]] + t[2]

def p_rules(t):
    '''rules :
             | rule rules'''
    if len(t) == 1:
        t[0] = []
    else:
        t[0] = [t[1]] + t[2]

def p_decl(t):
    '''decl : const constdecls
            | type typedecls
            | var vardecls'''
    if t[1] == 'const':
        t[0] = t[2]
    elif t[1] == 'type':
        t[0] = t[2]
    else:
        assert t[1] == 'var'
        t[0] = t[2]

def p_constdecls(t):
    '''constdecls :
                  | constdecl SEMI constdecls'''
    if len(t) == 1:
        t[0] = []
    else:
        t[0] = [t[1]] + t[3]

def p_typedecls(t):
    '''typedecls :
                 | typedecl SEMI typedecls'''
    if len(t) == 1:
        t[0] = []
    else:
        t[0] = [t[1]] + t[3]

def p_vardecls(t):
    '''vardecls :
                | vardecl SEMI vardecls'''
    if len(t) == 1:
        t[0] = []
    else:
        t[0] = [t[1]] + t[3]

def p_constdecl(t):
    '''constdecl : ID COLON expr'''
    t[0] = Constdecl(t[1], t[3])

def p_typedecl(t):
    '''typedecl : ID COLON typeExpr'''
    t[0] = Typedecl(t[1], t[3])

def p_typeExpr(t):
    '''typeExpr : ID
                | expr DOUBLEDOT expr
                | enum LBRACE IDs RBRACE
                | record vardecls end
                | record vardecls endrecord
                | array LSQUARE typeExpr RSQUARE of typeExpr'''
    if len(t) == 2:
        t[0] = Id(t[1])
    elif t[2] == '..':
        t[0] = Range(t[1], t[3])
    elif t[1] == 'enum':
        t[0] = Enum(t[3])
    elif t[1] == 'record':
        t[0] = Record(t[2])
    else:
        assert t[1] == 'array'
        t[0] = Array(t[3], t[6])

def p_vardecl(t):
    '''vardecl : IDs COLON typeExpr'''
    t[0] = Vardecl(t[1], t[3])

def p_procdecl(t):
    '''procdecl : procedureB
                | functionB'''
    t[0] = t[1]

def p_procedureB(t):
    '''procedureB : procedure ID LBRACKET oformals RBRACKET COLON header body end SEMI
                  | procedure ID LBRACKET oformals RBRACKET COLON header body endprocedure SEMI'''
    t[0] = Procedure(t[2], t[4], t[7], t[8])

def p_functionB(t):
    '''functionB : function ID LBRACKET oformals RBRACKET COLON header body end SEMI
                 | function ID LBRACKET oformals RBRACKET COLON header body endfunction SEMI'''
    t[0] = Function(t[2], t[4], t[7], t[8])

def p_oformals(t):
    '''oformals :
                | formals'''
    if len(t) == 1:
        t[0] = []
    else:
        t[0] = t[1]

def p_formals(t):
    '''formals : formal
               | formal SEMI formals'''
    if len(t) == 2:
        t[0] = [t[1]]
    else:
        t[0] = [t[1]] + t[3]

def p_header(t):
    '''header :
              | decls begin'''
    if len(t) == 1:
        t[0] = []
    else:
        t[0] = t[1]

def p_body(t):
    '''body :
            | stmts'''
    if len(t) == 1:
        t[0] = []
    else:
        t[0] = t[1]

def p_formal(t):
    '''formal : var IDs COLON typeExpr
              | IDs COLON typeExpr'''
    if len(t) == 5:
        t[0] = Formal(t[2], t[4])
    else:
        assert len(t) == 4
        t[0] = Formal(t[1], t[3])

def p_designator(t):
    '''designator : ID quals'''
    t[0] = Designator(t[1], t[2])

def p_quals(t):
    '''quals :
             | DOT ID quals
             | LSQUARE expr RSQUARE quals'''
    if len(t) == 1:
        t[0] = []
    elif len(t) == 4:
        t[0] = [QualID(t[2])] + t[3]
    else:
        t[0] = [QualIndex(t[2])] + t[3]

def p_expr(t):
    '''expr : LBRACKET expr RBRACKET
            | designator
            | integer_constant
            | ID LBRACKET actuals RBRACKET
            | forall quantifier do expr endforall
            | forall quantifier do expr end
            | exists quantifier do expr endexists
            | exists quantifier do expr end
            | expr ADD expr
            | expr SUB expr
            | expr MUL expr
            | expr DIV expr
            | expr MOD expr
            | NOT expr
            | expr OR expr
            | expr AND expr
            | expr IMPLIES expr
            | expr LT expr
            | expr LTE expr
            | expr GT expr
            | expr GTE expr
            | expr EQ expr
            | expr NEQ expr
            | expr QUESTION expr COLON expr'''
    if t[1] == '(':
        t[0] = t[2]
    elif isinstance(t[1], Designator):
        t[0] = t[1]
    elif isinstance(t[1], IntegerConstant):
        t[0] = t[1]
    elif isinstance(t[1], Id):
        t[0] = t[1]
    elif t[1] == 'forall':
        t[0] = Forall(t[2], t[4])
    elif t[1] == 'exists':
        t[0] = Exists(t[2], t[4])
    elif t[2] == '+':
        t[0] = Add(t[1], t[3])
    elif t[2] == '-':
        t[0] = Subtract(t[1], t[3])
    elif t[2] == '*':
        t[0] = Multiply(t[1], t[3])
    elif t[2] == '/':
        t[0] = Divide(t[1], t[3])
    elif t[2] == '%':
        t[0] = Modulus(t[1], t[3])
    elif t[1] == '!':
        t[0] = Not(t[2])
    elif t[2] == '|':
        t[0] = Or(t[1], t[3])
    elif t[2] == '&':
        t[0] = And(t[1], t[3])
    elif t[2] == '->':
        t[0] = Implies(t[1], t[3])
    elif t[2] == '<':
        t[0] = LessThan(t[1], t[3])
    elif t[2] == '<=':
        t[0] = LessThanOrEqual(t[1], t[3])
    elif t[2] == '>':
        t[0] = GreaterThan(t[1], t[3])
    elif t[2] == '>=':
        t[0] = GreaterThanOrEqual(t[1], t[3])
    elif t[2] == '=':
        t[0] = Equal(t[1], t[3])
    elif t[2] == '!=':
        t[0] = NotEqual(t[1], t[3])
    elif t[2] == '?':
        t[1] = TriCond(t[1], t[3], t[5])

def p_stmts(t):
    '''stmts : stmt ostmts'''
    t[0] = [t[1]] + t[2]

def p_ostmts(t):
    '''ostmts :
              | SEMI
              | SEMI stmt ostmts'''
    if len(t) in [1, 2]:
        t[0] = []
    else:
        t[0] = [t[2]] + t[3]

def p_stmt(t):
    '''stmt : assignment
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
            | returnstmt'''
    t[0] = t[1]

def p_assignment(t):
    '''assignment : designator ASSIGN expression'''
    t[0] = Assignment(t[1], t[3])

def p_ifstmt(t):
    '''ifstmt : if expr then body elsifs else body endif
              | if expr then body elsifs else body end
              | if expr then body elsifs endif
              | if expr then body elsifs end'''
    if len(t) == 9:
        t[0] = If(t[2], t[4], t[5], t[7])
    else:
        t[0] = If(t[2], t[4], t[5], [])

def p_elsifs(t):
    '''elsifs :
              | elsif expr then body elsifs'''
    if len(t) == 1:
        t[0] = []
    else:
        t[0] = [Elsif(t[2], t[4])] + t[5]

def p_switchstmt(t):
    '''switchstmt : switch expr cases else body endswitch
                  | switch expr cases else body end
                  | switch expr cases endswitch
                  | switch expr cases end'''
    if len(t) == 7:
        t[0] = Switch(t[2], t[3], t[5])
    else:
        t[0] = Switch(t[2], t[3], [])

def p_cases(t):
    '''cases :
             | case expr oexpr COLON body cases'''
    if len(t) == 1:
        t[0] = []
    else:
        t[0] = [Case([t[2]] + t[3], t[5])] + t[6]

def p_oexpr(t):
    '''oexpr :
             | COMMA expr oexpr'''
    if len(t) == 1:
        t[0] = []
    else:
        t[0] = [t[2]] + t[3]

def p_forstmt(t):
    '''forstmt : for quantifier do body endfor
               | for quantifier do body end'''
    t[0] = For(t[2], t[4])

def p_quantifier(t):
    '''quantifier : ID COLON typeExpr
                  | ID ASSIGN expr to expr
                  | ID ASSIGN expr to expr by expr'''
    if len(t) == 4:
        t[0] = QuantifierPrecise(t[1], t[3])
    elif len(t) == 6:
        t[0] = QuantifierImprecise(t[1], t[3], t[5], None)
    else:
        t[0] = QuantifierImprecise(t[1], t[3], t[5], t[7])

def p_whilestmt(t):
    '''whilestmt : while expr do body endwhile
                 | while expr do body end'''
    t[0] = While(t[2], t[4])

def p_aliasstmt(t):
    '''aliasstmt : alias aliasB oaliasB do body endalias
                 | alias aliasB oaliasB do body end'''
    t[0] = AliasStmt([t[2]] + t[3], t[5])

def p_oaliasB(t):
    '''oaliasB :
               | SEMI aliasB oaliasB'''
    if len(t) == 1:
        t[0] = []
    else:
        t[0] = [t[2]] + t[3]

def p_aliasB(t):
    '''aliasB : ID COLON expr'''
    t[0] = Alias(t[1], t[3])

def p_proccall(t):
    '''proccall : ID LBRACKET expr oexpr RBRACKET'''
    t[0] = Proccall(t[1], [t[3]] + t[4])

def p_clearstmt(t):
    '''clearstmt : clear designator'''
    t[0] = Clear(t[2])

def p_errorstmt(t):
    '''errorstmt : error STRING'''
    t[0] = Error(t[2])

def p_assertstmt(t):
    '''assertstmt : assert expr
                  | assert expr STRING'''
    if len(t) == 3:
        t[0] = Assert(t[2], None)
    else:
        t[0] = Assert(t[2], t[3])

def p_putstmt(t):
    '''putstmt : put expr
               | put STRING'''
    t[0] = Put(t[2])

def p_returnstmt(t):
    '''returnstmt : return
                  | return expr'''
    if len(t) == 2:
        t[0] = Return(None)
    else:
        t[0] = Return(t[2])

def p_rules(t):
    '''rules : rule orules
             | rule orules SEMI'''
    t[0] = [t[1]] + t[2]

def p_orules(t):
    '''orules :
              | SEMI rule orules'''
    if len(t) == 1:
        t[0] = []
    else:
        t[0] = [t[2]] + t[3]

def p_rule(t):
    '''rule : simplerule
            | startstateB
            | invariantB
            | rulesetB
            | aliasrule'''
    t[0] = t[1]

def p_simplerule(t):
    '''simplerule : rule guard header body endrule
                  | rule STRING guard header body endrule
                  | rule guard header body end
                  | rule STRING guard header body end'''
    if len(t) == 6:
        t[0] = SimpleRule(None, t[2], t[3], t[4])
    else:
        t[0] = SimpleRule(t[2], t[3], t[4], t[5])

def p_guard(t):
    '''guard :
             | expr IMPLICATION'''
    if len(t) == 1:
        t[0] = None
    else:
        t[0] = t[1]

def p_startstateB(t):
    '''startstateB : startstate header body endstartstate
                   | startstate header body end
                   | startstate STRING header body endstartstate
                   | startstate STRING header body end'''
    if len(t) == 5:
        t[0] = StartState(None, t[2], t[3])
    else:
        t[0] = StartState(t[2], t[3], t[4])

def p_invariantB(t):
    '''invariantB : invariant expr
                  | invariant STRING expr'''
    if len(t) == 3:
        t[0] = Invariant(None, t[2])
    else:
        t[0] = Invariant(t[2], t[3])

def p_rulesetB(t):
    '''rulesetB : ruleset quantifier oquantifiers do rules endruleset
                | ruleset quantifier oquantifiers do rules end'''
    t[0] = Ruleset([t[2]] + t[3], t[5])

def p_oquantifiers(t):
    '''oquantifiers :
                    | SEMI quantifier oquantifiers'''
    if len(t) == 1:
        t[0] = []
    else:
        t[0] = [t[2]] + t[3]

def p_aliasrule(t):
    '''aliasrule : alias aliasB oaliasB do rules endalias
                 | alias aliasB oaliasB do rules end'''
    t[0] = AliasRule([t[2]] + t[3], t[5])

def p_IDs(t):
    '''IDs : ID
           | ID COMMA IDs'''
    if len(t) == 2:
        t[0] = [t[1]]
    else:
        t[0] = [t[1]] + t[3]
