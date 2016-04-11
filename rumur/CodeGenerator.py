import itertools, six
from IR import Add, And, Assignment, BoolEq, IntEq, ErrorStmt, Expr, IfStmt, Imp, Or, Procedure, Program, PutStmt, ReturnStmt, VarRead, VarWrite, StateRead, StateWrite, Lit, ProcCall, SimpleRule, LT, GT, StartState, ClearStmt, ForStmt, TypeRange, Stmt

def mangle(name):
    return 'model_%s' % name

BASE, PROTOTYPES, FUNCTIONS, RULES = range(4)

MPZ, BOOL = range(2)

class Generator(object):

    def __init__(self, env):
        self.env = env
        self.sections = [[], [], [], []]

    def to_code(self, ir, lvalue=False):

        if isinstance(ir, Add):
            return ['st_add('] + self.to_code(ir.left) + [','] + self.to_code(ir.right) + [')']

        elif isinstance(ir, And):
            return ['('] + self.to_code(ir.left) + ['&&'] + self.to_code(ir.right) + [')']
        
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

        elif isinstance(ir, IntEq):
            return ['st_eq('] + self.to_code(ir.left) + [','] + self.to_code(ir.right) + [')']

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
                s += ['for(temp_st_t ', mangle(quan.symbol), '=st_new(', str(quan.typeexpr.lower), ');;st_inc(', mangle(quan.symbol), ')){']
                for stmt in ir.stmts:
                    s += self.to_code(stmt)
                s += ['if(st_eq(', mangle(quan.symbol), ',', str(quan.typeexpr.upper), '){break;}}']
                return s
            elif quan.typeexpr is not None:
                raise NotImplementedError
            else:
                assert quan.lower is not None
                assert quan.upper is not None
                s += ['for(temp_st_t ', mangle(quan.symbol), '=st_new(', str(quan.lower), ');;', mangle(quan.symbol), '=st_add(', mangle(quan.symbol), ',stnew(']
                if quan.step is None:
                    s.append('1')
                else:
                    s.append(str(quan.step))
                s += ['))){']
                for stmt in ir.stmts:
                    s += self.to_code(stmt)
                s += ['if(st_eq(', mangle(quan.symbol), ',', str(quan.upper), '){break;}}']
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

        elif isinstance(ir, Lit):
            return ['st_new(', str(ir.value), ')']

        elif isinstance(ir, LT):
            return ['st_lt('] + self.to_code(ir.left) + [','] + self.to_code(ir.right) + [')']

        elif isinstance(ir, Or):
            return ['('] + self.to_code(ir.left) + ['||'] + self.to_code(ir.right) + [')']

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

            for name, var in ir.parameters.vars.items():
                s += ['' if var.writable else 'const ', 'st_t ', mangle(name), ',']

            s += ['){']
            for name, var in ir.decls.constants.items():
                s += ['st_t ', mangle(name), ';do{int _t=mpz_init_set_str(', mangle(name), '.m,"', str(var.typ.value), '",10);assert(_t==0);}while(0);']
            for name, var in ir.decls.vars.items():
                assert var.writable
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

def to_code(scope, ir):
    g = Generator(scope)
    g.to_code(ir)
    yield itertools.chain(*g.sections)
