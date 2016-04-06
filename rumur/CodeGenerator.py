import itertools, six
from IR import Add, And, Assignment, Eq, ErrorStmt, Expr, IfStmt, Imp, Or, Procedure, Program, PutStmt, ReturnStmt, VarRead, VarWrite, StateRead, StateWrite, Lit, ProcCall, SimpleRule, LT, GT, StartState, ClearStmt, ForStmt, TypeRange, Stmt

def mangle(name):
    return 'model_%s' % name

BASE, PROTOTYPES, FUNCTIONS, RULES = range(4)

MPZ, BOOL = range(2)

class Generator(object):

    def __init__(self, env):
        self.env = env
        self.sections = [[], [], [], []]

    def binop(self, ir, func):
        return ['({st_t _t1='] + self.to_code(ir.left) + [';temp_st_t _t2='] + self.to_code(ir.right) + [';', func, '(_t1.m,_t1.m,_t2.m);_t1;})']

    def to_code(self, ir, lvalue=False):

        if isinstance(ir, Add):
            return self.binop(ir, 'mpz_add')

        elif isinstance(ir, And):
            return ['({temp_st_t _t1='] + self.to_code(ir.left) + [';temp_st_t _t2='] + self.to_code(ir.right) + [';(!!mpz_cmp_ui(_t1.m,0))&&(!!mpz_cmp_ui(_t2.m,0));})']
        
        elif isinstance(ir, Assignment):
            s = ['mpz_set_ui(']
            if isinstance(ir.designator, StateWrite):
                raise NotImplementedError
            else:
                assert isinstance(ir.designator, VarWrite)
                s += [mangle(ir.designator.root)]
                # TODO: stems
            s += [',('] + self.to_code(ir.expr) + ['));']
            return s

        elif isinstance(ir, ClearStmt):
            return ['mpz_set_ui(', ir.designator, '.m,0);']

        elif isinstance(ir, Eq):
            if ir.left.result_type != ir.right.result_type:
                raise RumurError('comparison between expressions of mismatched type')
            elif ir.left.result_type == bool:
                return ['('] + self.to_code(ir.left) + ['=='] + self.to_code(ir.right) + [')']
            else:
                assert ir.left.result_type == int
                return ['({temp_st_t _t1='] + self.to_code(ir.left) + [';temp_st_t _t2='] + self.to_code(ir.right) + [';mpz_cmp(_t1.m,_t2.m)==0;})']

        elif isinstance(ir, ErrorStmt):
            if ir.string is None:
                msg = '<unnamed>'
            else:
                msg = ir.string
            return ['rumur_error("', msg, '");']

        elif isinstance(ir, ForStmt):
            scope, quan = ir.quantifier
            s = []
            if isinstance(quan.typeexpr, TypeRange):
                s += ['for(temp_st_t ', mangle(quan.symbol), '='] + self.to_code(quan.typeexpr.lower) + [';({temp_st_t _t='] + self.to_code(quan.typeexpr.upper) + [';mpz_cmp(', mangle(quan.symbol), '.m,_t.m)>=0;});mpz_add_ui(', mangle(quan.symbol), '.m,1)){']
                for stmt in ir.stmts:
                    s += self.to_code(stmt)
                s += ['}']
                return s
            elif quan.typeexpr is not None:
                raise NotImplementedError
            else:
                assert quan.lower is not None
                assert quan.upper is not None
                s += ['for(temp_st_t ', mangle(quan.symbol), '='] + self.to_code(quan.lower) + [';({temp_st_t _t='] + self.to_code(quan.upper) + [';mpz_cmp(', mangle(quan.symbol), '.m,_t.m)>=0;});']
                if quan.step is None:
                    s += ['mpz_add_ui(', mangle(quan.symbol), '.m,1)']
                else:
                    s += ['({temp_st_t _t='] + self.to_code(quan.step) + [';mpz_add(', mangle(quan.symbol), '.m,_t.m);})']
                s += ['){']
                for stmt in ir.stmts:
                    s += self.to_code(stmt)
                s += ['}']
                return s

        elif isinstance(ir, IfStmt):
            s = []
            for index, branch in enumerate(ir.branches):
                if index == 0:
                    s += ['if']
                elif branch.cond is not None:
                    s += ['else if']
                else:
                    s += ['else']

                if branch.cond is not None:
                    s += ['('] + self.to_code(branch.cond) + [')']

                s += ['{']
                for stmt in branch.stmts:
                    s += self.to_code(stmt)
                s += ['}']
            return s

        elif isinstance(ir, GT):
            return ['({temp_st_t _t1='] + self.to_code(ir.left) + [';temp_st_t _t2='] + self.to_code(ir.right) + [';mpz_cmp(_t1.m,_t2.m)>0;})']

        elif isinstance(ir, Imp):
            return ['({temp_st_t _t1='] + self.to_code(ir.left) + [';temp_st_t _t2='] + self.to_code(ir.right) + [';(!mpz_cmp_ui(_t1.m,0))||(!!mpz_cmp_ui(_t2.m,0));})']

        elif isinstance(ir, Lit):
            return ['({st_t _t1;int _t2=mpz_init_set_str(_t1.m,"', str(ir.value), '",10);assert(_t2==0);_t1;})']

        elif isinstance(ir, LT):
            return ['({temp_st_t _t1='] + self.to_code(ir.left) + [';temp_st_t _t2='] + self.to_code(ir.right) + [';mpz_cmp(_t1.m,_t2.m)<0;})']

        elif isinstance(ir, Or):
            return ['({temp_st_t _t1='] + self.to_code(ir.left) + [';temp_st_t _t2='] + self.to_code(ir.right) + [';(!!mpz_cmp_ui(_t1.m,0))||(!!mpz_cmp_ui(_t2.m,0));})']

        elif isinstance(ir, ProcCall):
            s = [mangle(ir.symbol), '(']
            for expr in ir.exprs:
                s += ['('] + self.to_code(expr) + ['),']
            s += [');']
            return s

        elif isinstance(ir, Procedure):
            s = ['void ', mangle(ir.name), '(']

            # The scope for procedure parameters should contain only variables.
            assert len(ir.parameters.constants) == 0
            assert len(ir.parameters.types) == 0

            for name, (typ, writable) in ir.parameters.vars.items():
                s += ['' if writable else 'const ', 'st_t ', mangle(name), ',']

            s += ['){']
            for name, value in ir.decls.constants.items():
                s += ['st_t ', mangle(name), ';do{int _t=mpz_init_set_str(', mangle(name), '.m,"', str(value), '",10);assert(_t==0);}while(0);']
            for name, (typ, writable) in ir.decls.vars.items():
                assert writable
                s += ['st_t ', mangle(name), ';mpz_init(', mangle(name), '.m);']

            for stmt in ir.stmts:
                s += self.to_code(stmt)
            s += ['}']
            return s

        elif isinstance(ir, Program):
            generators = []
            for p in ir.procs:
                generators.append(self.to_code(p))
            for r in ir.rules:
                generators.append(self.to_code(r))
            return itertools.chain(*generators)

        elif isinstance(ir, PutStmt):
            if isinstance(ir.arg, Expr):
                if ir.arg.result_type == bool:
                    return ['printf('] + self.to_code(ir.arg) + ['?"true":"false");']
                assert ir.arg.result_type == int
                return ['do{temp_st_t _t='] + self.to_code(ir.arg) + [';mpz_out_str(stdout,10,_t.m);}while(0);']
            assert isinstance(ir.arg, six.string_types)
            return ['printf("%s","', ir.arg, '");']

        elif isinstance(ir, ReturnStmt):
            if ir.value is None:
                return ['return;']
            return ['return '] + self.to_code(ir.value) + [';']

        elif isinstance(ir, SimpleRule):
            index = len(self.sections[RULES])
            self.sections[RULES].extend(
                ['{"', ir.string, '",', 'NULL' if ir.expr is None else 'guard%d' % index, ',rule', str(index), '},'])
            s = []

            if ir.expr is not None:
                s += ['bool guard', str(index), '(const state_t s){return '] + self.to_code(ir.expr) + [';}']

            s += ['void rule', str(index), '(state_t s){']
            for name, value in ir.decls.constants.items():
                s += ['temp_st_t ', mangle(name), ';do{int _t=mpz_init_set_str(', mangle(name), '.m,"', str(value), '",10);assert(_t==0);}while(0);']
            for name, (typ, writable) in ir.decls.vars.items():
                assert writable
                s += ['temp_st_t ', mangle(name), ';mpz_init(', mangle(name), '.m);']

            for stmt in ir.stmts:
                s += self.to_code(stmt)

            s += ['}']
            return s

        elif isinstance(ir, StartState):
            s = ['static const char*start_state_name="', ir.string, '";void start(state_t s){']
            for name, value in ir.decls.constants.items():
                s += ['temp_st_t ', mangle(name), ';do{int _t=mpz_init_set_str(', mangle(name), '.m,"', str(value), '",10);assert(_t==0);}while(0);']
            for name, (typ, writable) in ir.decls.vars.items():
                assert writable
                s += ['temp_st_t ', mangle(name), ';mpz_init(', mangle(name), '.m);']

            for stmt in ir.stmts:
                s += self.to_code(stmt)

            s += ['}']
            return s

        elif isinstance(ir, VarRead):
            return [mangle(ir.root)]
            #TODO: stems

        raise NotImplementedError('code generation for %s' % str(type(ir)))

    def from_expr(self, e):
        assert isinstance(e, Expr)

        if isinstance(e, Add):
            ltyp, l = self.from_expr(e.left)
            assert ltyp == MPZ
            rtyp, r = self.from_expr(e.right)
            assert rtyp == MPZ
            return MPZ, '({mpz _t1=%s;temp_mpz _t2=%s;mpz_add(_t1.m,_t1.m,_t2.m);_t1;})' % (l, r)

        elif isinstance(e, And):
            ltyp, l = self.from_expr(e.left)
            assert ltyp == BOOL
            rtyp, r = self.from_expr(e.right)
            assert rtyp == BOOL
            return BOOL, '(%s&&%s)' % (l, r)

        elif isinstance(e, Eq):
            ltyp, l = self.from_expr(e.left)
            rtyp, r = self.from_expr(e.right)
            assert ltyp == rtyp
            if ltyp == MPZ:
                return BOOL, '({temp_mpz _t1=%s;temp_mpz _t2=%s;mpz_cmp(_t1.m,_t2.m)==0;})' % (l, r)
            return BOOL, '(%s==%s)' % (l, r)

        elif isinstance(e, GT):
            ltyp, l = self.from_expr(e.left)
            assert ltyp == MPZ
            rtyp, r = self.from_expr(e.right)
            assert rtyp == MPZ
            return BOOL, '({temp_mpz _t1=%s;temp_mpz _t2=%s;mpz_cmp(_t1.m,_t2.m)>0;})' % (l, r)

        elif isinstance(e, Lit):
            return MPZ, '({mpz _t;mpz_init_set_str(_t.m,"%s",10);_t;})' % e.value

        raise NotImplementedError('code generation for %s' % str(type(e)))

    def from_stmt(self, s):
        assert isinstance(s, Stmt)

        if isinstance(s, PutStmt):
            if isinstance(s.arg, Expr):
                t, v = self.from_expr(s.arg)
                if t == BOOL:
                    return 'printf(%s?"true":"false");' % v
                assert t == MPZ
                return 'do{temp_st_t _t=%s;mpz_out_str(stdout,10,_t.m);}while(0);' % v
            assert isinstance(s.arg, six.string_types)
            return 'fputs("%s",stdout);' % s.arg

        raise NotImplementedError('code generation for %s' % str(type(e)))

def to_code(scope, ir):
    g = Generator(scope)
    g.to_code(ir)
    yield itertools.chain(*g.sections)




# from functools import partial
# from Access import DirectSymbol, StateSymbol
# from Type import Array, Enum, Range, Record, Type
# from MPZ import Literal, MPTerm
# 
# class Generator(object):
#     def generate(self, n):
# 
#         if n.head == 'constdecl':
#             name = str(n.tail[0])
#             expr = self.generate(n.tail[1])
#             return ConstDecl(name, expr)
# 
#         elif n.head == 'decl':
#             return [self.generate(t) for t in n.tail]
# 
#         elif n.head == 'expr':
# 
#             if n.tail[0].head == 'lbrace':
#                 assert n.tail[2].head == 'rbrace'
#                 return self.generate(n.tail[1])
# 
#             elif n.tail[0].head == 'integer_constant':
#                 return Lit(int(str(n.tail[0])))
# 
#             elif n.tail[0].head == 'symbol':
#                 assert n.tail[1].head == 'lbrace'
#                 assert n.tail[3].head == 'rbrace'
#                 yield str(n.tail[0])
#                 yield from bracket(self.generate(n.tail[2]))
# 
#             elif n.tail[0].head == 'forall':
#                 ...
# 
#             elif n.tail[0].head == 'exists':
#                 ...
# 
#             elif n.tail[1].head == 'add':
#                 left = self.generate(n.tail[0])
#                 right = self.generate(n.tail[2])
#                 return Add(left, right)
# 
#             elif n.tail[1].head == 'sub':
#                 yield from bin_op(n.tail[0], '-', n.tail[2])
# 
#             elif n.tail[1].head == 'mul':
#                 yield from bin_op(n.tail[0], '*', n.tail[2])
# 
#             elif n.tail[1].head == 'div':
#                 yield from bin_op(n.tail[0], '/', n.tail[2])
# 
#             elif n.tail[1].head == 'mod':
#                 left = self.generate(n.tail[0])
#                 right = self.generate(n.tail[2])
#                 return Mod(left, right)
# 
#             elif n.tail[0].head == 'not':
#                 operand = self.generate(n.tail[1])
#                 return Not(operand)
# 
#             elif 
# 
# def generate(env, n):
# 
#         elif n.tail[0].head == 'forall':
#             return hol_operation(env, 'forall', n.tail[1], n.tail[2])
# 
#         elif n.tail[0].head == 'exists':
#             return hol_operation(env, 'exists', n.tail[1], n.tail[2])
# 
#         elif n.tail[1].head == 'or':
#             operand1 = generate(env, n.tail[0])
#             operand2 = generate(env, n.tail[2])
#             return operand1 | operand2
# 
#         elif n.tail[1].head == 'and':
#             operand1 = generate(env, n.tail[0])
#             operand2 = generate(env, n.tail[2])
#             return operand1 & operand2
# 
#         elif n.tail[1].head =='implies':
#             operand1 = generate(env, n.tail[0])
#             operand2 = generate(env, n.tail[2])
#             return operand1.implies(operand2)
# 
#         elif n.tail[1].head == 'lt':
#             operand1 = generate(env, n.tail[0])
#             operand2 = generate(env, n.tail[2])
#             return operand1 < operand2
# 
#         elif n.tail[1].head == 'lte':
#             operand1 = generate(env, n.tail[0])
#             operand2 = generate(env, n.tail[2])
#             return operand1 <= operand2
# 
#         elif n.tail[1].head == 'gt':
#             operand1 = generate(env, n.tail[0])
#             operand2 = generate(env, n.tail[2])
#             return operand1 > operand2
# 
#         elif n.tail[1].head == 'gte':
#             operand1 = generate(env, n.tail[0])
#             operand2 = generate(env, n.tail[2])
#             return operand1 >= operand2
# 
#         elif n.tail[1].head == 'eq':
#             operand1 = generate(env, n.tail[0])
#             operand2 = generate(env, n.tail[2])
#             return operand1 == operand2
# 
#         elif n.tail[1].head == 'neq':
#             operand1 = generate(env, n.tail[0])
#             operand2 = generate(env, n.tail[2])
#             return operand1 != operand2
# 
#         elif n.tail[1].head == 'question':
#             operand1 = generate(env, n.tail[0])
#             operand2 = generate(env, n.tail[2])
#             operand3 = generate(env, n.tail[4])
#             return operand1.tricond(operand2, operand3)
# 
#         raise NotImplementedError
# 
#     elif n.head == 'function':
#         # The [1:-1] slices below are because we know the first and last token
#         # of a function production.
#         symbol = n.tail[0]
#         formals = filter(lambda x: x.head == 'formal', n.tail[1:-1])
#         typeexpr = filter(lambda x: x.head == 'typeexpr', n.tail[1:-1])
#         assert len(typeexpr) == 1
#         return_type = typeexpr[0]
#         decls = filter(lambda x: x.head == 'decl', n.tail[1:-1])
#         body = filter(lambda x: x.head == 'stmts', n.tail[1:-1])
#         stmts = body[0] if len(body) > 0 else None
#         return make_function(env, symbol, formals, return_type, decls, stmts)
# 
#     elif n.head == 'integer_constant':
#         value = int(n.tail[0])
#         return Literal(value)
# 
#     elif n.head == 'start':
#         if len(n.tail) != 1:
#             raise Exception('multiple initial tokens')
#         return generate(env, n.tail[0])
# 
#     elif n.head == 'procdecl':
#         return generate(env, n.tail[0])
# 
#     elif n.head == 'procedure':
#         # See comments for 'function'
#         symbol = n.tail[0]
#         formals = filter(lambda x: x.head == 'formal', n.tail[1:-1])
#         return_type = None
#         decls = filter(lambda x: x.head == 'decl', n.tail[1:-1])
#         body = filter(lambda x: x.head == 'stmts', n.tail[1:-1])
#         stmts = body[0] if len(body) > 0 else None
#         return make_function(env, symbol, formals, return_type, decls, stmts)
# 
#     elif n.head == 'program':
#         env.new_scope()
#         return concat(env, n.tail)
# 
#     elif n.head == 'putstmt':
#         if n.tail[0].head == 'expr':
#             expr = generate(env, n.tail[0]).to_mpz()
#             return 'do {\n' \
#                    '  mpz_t x = %(expr)s;\n' \
#                    '  gmp_printf("%%Zu", x);\n' \
#                    '  mpz_clear(x);\n' \
#                    '} while (0);' % locals()
#         else:
#             assert n.tail[0].head == 'string'
#             s = n.tail[0].tail[0][1:-1] # strip quotes
#             # XXX: Escape this safely.
#             return 'printf("%%s", "%(s)s");' % locals()
# 
#     elif n.head == 'returnstmt':
#         code = ''
#         if len(n.tail) > 0:
#             expr = generate(env, n.tail[0]).to_mpz()
#             # Note that we assume the variable 'ret' is available.
#             code += 'do {\n' \
#                     '  mpz_t x = %(expr)s;\n' \
#                     '  mpz_set(ret, x);\n' \
#                     '  mpz_clear(x);\n' \
#                     '} while (0);\n' % locals()
#         # See make_function() for where this label is emitted.
#         code += 'goto coda;'
#         return code
# 
#     elif n.head == 'symbol':
#         return mangle(n.tail[0])
# 
#     elif n.head == 'typedecl':
#         symbol = generate(env, n.tail[0])
#         type = decode_type(env, n.tail[1])
#         env.typedef(symbol, type)
#         return '' # No code required
# 
#     elif n.head == 'vardecl':
#         code = ''
#         type = decode_type(env, n.tail[-1])
#         for s in n.tail[:-1]:
#             symbol = generate(env, s)
#             env.declare(symbol, type)
#             if not env.bare():
#                 # We only need to generate code if this is not a state variable.
#                 code += 'mpz_t %(symbol)s;\n' \
#                         'mpz_init(%(symbol)s);' % locals()
#         return code
# 
#     elif n.head == 'whilestmt':
#         expr = generate(env, n.tail[0]).to_mpz()
#         if n.tail[1].head == 'stmts':
#             stmts = cat(env, n.tail[1].tail)
#         else:
#             stmts = ''
#         return 'while (({\n' \
#                '    mpz_t x = %(expr)s;\n' \
#                '    int r;\n' \
#                '    r = (mpz_cmp_ui(x, 0) != 0);\n' \
#                '    mpz_clear(x);\n' \
#                '    r;\n' \
#                '  })) {\n' \
#                '  %(stmts)s\n' \
#                '}' % locals()
# 
#     else:
#         raise NotImplementedError
# 
# def hol_operation(env, binder, quantifier, body):
# 
#     if binder == 'forall':
#         initial_value = 1
#         modifier = '!'
#         abort_value = 0
#     else:
#         assert binder == 'exists'
#         initial_value = 0
#         modifier = ''
#         abort_value = 1
# 
#     iterator = generate(env, quantifier.tail[0])
#     if quantifier.tail[1].head == 'typeexpr':
#         # First case of quantifier
#         lower_init = 'mpz_init_set_ui(lower, 0);' #TODO
#         upper_init = 'mpz_init_set_ui(upper, 0);' #TODO
#         increment_init = 'mpz_init_set_ui(increment, 1);'
#     else:
#         l = generate(env, quantifier.tail[1])
#         lower_init = 'mpz_init_set(lower, %(l)s);' % locals()
#         u = generate(env, quantifier.tail[2])
#         upper_init = 'mpz_init_set(upper, %(u)s);' % locals()
#         if len(quantifier.tail) == 4:
#             # We have an explicit step clause
#             i = generate(env, quantifier.tail[3])
#             increment_init = 'mpz_init_set(increment, %(i)s);' % locals()
#         else:
#             increment_init = 'mpz_init_set_ui(increment, 1);'
# 
#     # Adjust indentation in case any of these are more than one line.
#     lower_init = hang(lower_init)
#     upper_init = hang(upper_init)
#     increment_init = hang(increment_init)
# 
#     expr = generate(env, body).to_mpz()
# 
#     return '({\n' \
#            '  mpz_t %(iterator)s;\n' \
#            '  mpz_t r;\n' \
#            '  mpz_set_ui(r, %(initial_value)d);\n' \
#            '  mpz_t lower, upper, increment;\n' \
#            '  %(increment_init)s\n' \
#            '  %(lower_init)s\n' \
#            '  %(upper_init)s\n' \
#            '  for (mpz_init_set(%(iterator)s, lower);\n' \
#            '       mpz_cmp(%(iterator)s, upper) < 0;\n' \
#            '       mpz_add(%(iterator)s, increment)) {\n' \
#            '    mpz_t x = %(expr)s;\n' \
#            '    if (%(modifier)s(mpz_cmp_ui(x, 0) != 0)) {\n' \
#            '      mpz_clear(x);\n' \
#            '      mpz_set_ui(r, %(abort_value)d);\n' \
#            '      break;\n' \
#            '    }\n' \
#            '    mpz_clear(x);\n' \
#            '  }\n' \
#            '  mpz_clear(%(iterator)s);\n' \
#            '  mpz_clear(%(lower)s);\n' \
#            '  mpz_clear(%(upper)s);\n' \
#            '  mpz_clear(%(increment)s);\n' \
#            '  r;\n' \
#            '})' % locals()
# 
# def decode_type(env, typeexpr):
#     if typeexpr.tail[0].head == 'symbol':
#         symbol = generate(env, typeexpr.tail[0])
#         referent = env.lookup(symbol)
#         if referent is None:
#             raise Exception('type alias refers to non-existent type %s' % typeexpr.tail[0].head)
#         elif not isinstance(referent, Type):
#             raise Exception('type alias refers to %s which is not a type' % typeexpr.tail[0].head)
#         return referent
# 
#     elif typeexpr.tail[0].head == 'expr':
#         min = generate(env, typeexpr.tail[0])
#         max = generate(env, typeexpr.tail[1])
#         return Range(min, max)
# 
#     elif typeexpr.tail[0].head == 'enum':
#         members = map(partial(generate, env), typeexpr.tail[1:])
#         return Enum(members)
# 
#     elif typeexpr.tail[0].head == 'record':
#         members = {}
#         offset = Literal(1)
#         for vardecl in typeexpr.tail[1:-1]:
#             type = decode_type(env, vardecl.tail[-1])
#             cardinality = type.cardinality()
#             for symbol in vardecl.tail[:-1]:
#                 s = generate(env, symbol)
#                 if s in members:
#                     raise Exception('duplicate record members %s given' % symbol.tail[0].head)
#                 members[s] = (offset, type)
#                 offset *= cardinality
#         return Record(members)
# 
#     else:
#         assert typeexpr.tail[0].head == 'array'
#         index_type = decode_type(env, typeexpr.tail[1])
#         member_type = decode_type(env, typeexpr.tail[2])
#         return Array(index_type, member_type)
# 
# def make_function(env, symbol, formals, return_type, decls, stmts):
#     function = generate(env, symbol)
#     env.new_scope()
#     params = []
#     if return_type is not None:
#         params.append('mpz_t ret')
#     prelude = []
#     coda = []
#     for formal in formals:
#         if formal.tail[0].head == 'var':
#             writable = True
#             symbols = formal.tail[1:-1]
#         else:
#             writable = False
#             symbols = formal.tail[:-1]
#         type = decode_type(env, formal.tail[-1])
#         for s in symbols:
#             mangled = generate(env, s)
#             env.declare(mangled, type)
#             p = 'p_%s' % mangled
#             params.append('mpz_t %s' % p)
#             if writable:
#                 prelude.append('mpz_t %(mangled)s = %(p)s;' % locals())
#             else:
#                 prelude.append('mpz_t %s;' % mangled)
#                 prelude.append('mpz_set(%(mangled)s, %(p)s);' % locals())
#                 coda.append('mpz_clear(%s);' % mangled)
# 
#     for decl in decls:
#         code = generate(env, decl)
#         # XXX: What happens if the decl calls mpz_init and we need to clear this later?
#         if code:
#             prelude.append(code)
# 
#     if stmts is None:
#         body = ''
#     else:
#         body = generate(env, stmts)
# 
#     prelude = '\n  '.join(prelude)
#     params = ', '.join(params)
#     coda = '\n  '.join(coda)
#     env.pop_scope()
#     return 'void %(function)s(%(params)s) {\n' \
#            '  %(prelude)s\n' \
#            '  %(body)s\n' \
#            'coda:\n' \
#            '  %(coda)s\n' \
#            '}' % locals()
# 
# def pinpoint(env, designator):
#     root = generate(env, designator.tail[0])
#     sym = env.lookup(root)
#     if isinstance(sym, StateSymbol):
#         assert env.state_variable is not None, \
#             'looking up \'%s\' which needs to be accessed through the state ' \
#             'in a context in which the state is unavailable'
#         origin = env.state_variable
#         offset = sym.offset
#     elif isinstance(sym, DirectSymbol):
#         origin = root
#         offset = Literal(1)
#     else:
#         raise Exception('attempt to locate something that is not a variable reference')
#     active_type = sym.type
#     for s in designator.tail[1:]:
#         if s.head == 'symbol':
#             if not isinstance(active_type, Record):
#                 raise Exception('accessing a member of something that is not a record')
#             member = generate(env, s)
#             if member not in active_type.members:
#                 raise Exception('access of non-existing record member %s' % s.tail[0])
#             factor, active_type = active_type.members[member]
#             offset *= factor
#         else:
#             assert s.head == 'expr'
#             if not isinstance(active_type, Array):
#                 raise Exception('accessing an index of something that is not an array')
#             expr = generate(env, s)
#             member_cardinality = active_type.member_type.cardinality()
#             active_type = active_type.member_type
#             offset *= member_cardinality * expr
#     cardinality = active_type.cardinality()
#     return origin, offset, cardinality
# 
# def concat(env, xs):
#     s = ''
#     for x in xs:
#         s += '%s\n\n' % generate(env, x)
#     if s != '':
#         s = s[:-2]
#     return s
# 
# def cat(env, xs):
#     s = ''
#     for x in xs:
#         s += '%s\n' % generate(env, x)
#     if s != '':
#         s = s[:-2]
#     return s
# 
# def mangle(symbol):
#     return 'user_%s' % symbol
# 
# def hang(block):
#     lines = block.split('\n')
#     indented = lines[0]
#     if len(lines) > 1:
#         indented += '\n%s' % '\n'.join(map(lambda x: '  %s' % x, lines[1:]))
#     return indented
