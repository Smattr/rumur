from OptCF import constant_fold
from Environment2 import Environment
from IR import Add, AliasRule, And, Assignment, Branch, ClearStmt, BoolEq, IntEq, Exists, Forall, ForStmt, GT, IfStmt, Imp, Invariant, Lit, LT, Method, Not, ProcCall, Procedure, Program, Quantifier, RuleSet, SimpleRule, StartState, Sub, TriCond, TypeArray, TypeEnum, TypeRange, TypeConstant, Mul, Guard, Designator, Bool
from RumurError import RumurError

class Generator(object):

    def __init__(self):
        self.env = Environment()

    def to_ir(self, node, lvalue=False):

        if node.head == 'assignment':
            designator = self.to_ir(node.tail[0], lvalue=True)
            expr = self.to_ir(node.tail[1])
            return Assignment(designator, expr, node)

        elif node.head == 'clearstmt':
            designator = self.to_ir(node.tail[0], lvalue=True)
            return ClearStmt(designator, node)

        elif node.head == 'constdecl':
            name = self.to_ir(node.tail[0])
            expr = self.to_ir(node.tail[1])
            if not isinstance(expr, Lit):
                raise RumurError('constant declaration is not constant', node)
            self.env.declare_constant(name, expr.value)
            return None

        elif node.head == 'designator':
            root = self.to_ir(node.tail[0])
            try:
                var = self.env.lookup_var(root)
            except KeyError:
                raise RumurError('designator %s refers to an undefined variable' % root, node)
            if lvalue and not var.writable:
                raise RumurError('attempt to write to read-only variable', node)
            if isinstance(var.typ, TypeConstant):
                if len(node.tail) > 1:
                    raise RumurError('excess qualifiers after reference to a constant', node)
                if lvalue:
                    raise RumurError('attempted assignment to a constant', node)
                return Lit(var.typ.value, node)
            stems = []
            result_type = var.typ
            for t in node.tail[1:]:
                if t.head == 'symbol':
                    sym = self.to_ir(t)
                    if not isinstance(result_type, TypeRecord):
                        raise RumurError('%s is not a member of the preceding expression' % sym, t)
                    try:
                        member = result_type.lookup_var(sym)
                    except KeyError:
                        raise RumurError('%s is not a member of the preceding expression' % sym, t)
                    stems.append(sym)
                    result_type = member.typ
                else:
                    assert t.head == 'expr'
                    if not isinstance(result_type, TypeArray):
                        raise RumurError('array index on a type that is not an array', t)
                    stems.append(self.to_ir(t))
                    result_type = result_type.member_type
            return Designator(var.offsetof is not None, root, stems, result_type, node)

        elif node.head == 'expr':
            if node.tail[0].head == 'expr1':
                return self.to_ir(node.tail[0])
            cond = self.to_ir(node.tail[0])
            if cond.result_type is not Bool:
                raise RumurError('tri-conditional condition does not evaluate to a boolean', node)
            casetrue = self.to_ir(node.tail[2])
            casefalse = self.to_ir(node.tail[4])
            if casetrue.result_type is not casefalse.result_type:
                raise RumurError('tri-conditional branches are not of the same type', node)
            return TriCond(cond, casetrue, casefalse, node)

        elif node.head == 'expr1':
            if node.tail[0].head == 'expr2':
                return self.to_ir(node.tail[0])
            left = self.to_ir(node.tail[0])
            if left.result_type is not Bool:
                raise RumurError('left operand to implies does not evaluate to a boolean', node)
            right = self.to_ir(node.tail[2])
            if right.result_type is not Bool:
                raise RumurError('right operand to implies does not evaluate to a boolean', node)
            return Imp(left, right, node)

        elif node.head == 'expr2':
            if node.tail[0].head == 'expr3':
                return self.to_ir(node.tail[0])
            raise NotImplementedError

        elif node.head == 'expr3':
            if node.tail[0].head == 'expr4':
                return self.to_ir(node.tail[0])

            assert node.tail[1].head == 'and'
            left = self.to_ir(node.tail[0])
            if left.result_type is not Bool:
                raise RumurError('left operand to and does not evaluate to a boolean', node)
            right = self.to_ir(node.tail[2])
            if right.result_type is not Bool:
                raise RumurError('right operand to and does not evaluate to a boolean', node)
            return And(left, right, node)

        elif node.head == 'expr4':
            if node.tail[0].head == 'expr5':
                return self.to_ir(node.tail[0])

            assert node.tail[0].head == 'not'
            operand = self.to_ir(node.tail[1])
            if operand.result_type is not Bool:
                raise RumurError('operand to not does not evaluate to a boolean', node)
            return Not(operand, node)

        elif node.head == 'expr5':
            if node.tail[0].head == 'expr6':
                return self.to_ir(node.tail[0])

            left = self.to_ir(node.tail[0])
            right = self.to_ir(node.tail[2])

            if node.tail[1].head == 'lt':
                if not isinstance(left.result_type, TypeRange):
                    raise RumurError('left operand to less than does not evaluate to an integer', node)
                if not isinstance(right.result_type, TypeRange):
                    raise RumurError('right operand to less than does not evaluate to an integer', node)
                return LT(left, right, node)

            elif node.tail[1].head == 'eq':
                if left.result_type is not right.result_type:
                    raise RumurError('equality comparison between expressions of differing types', node)
                if left.result_type is Bool:
                    return BoolEq(left, right, node)
                assert isinstance(left.result_type, TypeRange)
                return IntEq(left, right, node)

            elif node.tail[1].head == 'gt':
                if not isinstance(left.result_type, TypeRange):
                    raise RumurError('left operand to greater than does not evaluate to an integer', node)
                if not isinstance(right.result_type, TypeRange):
                    raise RumurError('right operand to greater than does not evaluate to an integer', node)
                return GT(left, right, node)

        elif node.head == 'expr6':
            if node.tail[0].head == 'expr7':
                return self.to_ir(node.tail[0])

            if node.tail[1].head == 'add':
                left = self.to_ir(node.tail[0])
                if not isinstance(left.result_type, TypeRange):
                    raise RumurError('left operand to addition does not evaluate to an integer', node)
                right = self.to_ir(node.tail[2])
                if not isinstance(right.result_type, TypeRange):
                    raise RumurError('right operand to addition does not evaluate to an integer', node)
                return Add(left, right, node)

            assert node.tail[1].head == 'sub'
            left = self.to_ir(node.tail[0])
            if not isinstance(left.result_type, TypeRange):
                raise RumurError('left operand to subtraction does not evaluate to an integer', node)
            right = self.to_ir(node.tail[2])
            if not isinstance(right.result_type, TypeRange):
                raise RumurError('right operand to subtraction does not evaluate to an integer', node)
            return Sub(left, right, node)

        elif node.head == 'expr7':
            if node.tail[0].head == 'expr8':
                return self.to_ir(node.tail[0])

            if node.tail[1].head == 'mul':
                left = self.to_ir(node.tail[0])
                if not isinstance(left.result_type, TypeRange):
                    raise RumurError('left operand to multiplication does not evaluate to an integer', node)
                right = self.to_ir(node.tail[2])
                if not isinstance(right.result_type, TypeRange):
                    raise RumurError('right operand to multiplication does not evaluate to an integer', node)
                return Mul(left, right, node)

            elif node.tail[1].head == 'div':
                left = self.to_ir(node.tail[0])
                if not isinstance(left.result_type, TypeRange):
                    raise RumurError('left operand to division does not evaluate to an integer', node)
                right = self.to_ir(node.tail[2])
                if not isinstance(right.result_type, TypeRange):
                    raise RumurError('right operand to division does not evaluate to an integer', node)
                return Div(left, right, node)

        elif node.head == 'expr8':

            if node.tail[0].head == 'designator':
                return self.to_ir(node.tail[0], lvalue=False)

            elif node.tail[0].head == 'integer_constant':
                return self.to_ir(node.tail[0])

            elif node.tail[0].head == 'forall':
                self.env.open_scope()
                quan = self.to_ir(node.tail[1])
                expr = self.to_ir(node.tail[2])
                quantifier = (self.env.scope, quan)
                self.env.close_scope()
                return Forall(quantifier, expr, node)

            elif node.tail[0].head == 'exists':
                self.env.open_scope()
                quan = self.to_ir(node.tail[1])
                expr = self.to_ir(node.tail[2])
                quantifier = (self.env.scope, quan)
                self.env.close_scope()
                return Exists(quantifier, expr, node)

        elif node.head == 'formal':
            if node.tail[0].head == 'var':
                writable = True
                remaining = node.tail[1:]
            else:
                writable = False
                remaining = node.tail

            typeexpr = self.to_ir(remaining[-1])

            for symbol in remaining[:-1]:
                name = self.to_ir(symbol)
                self.env.declare_var(name, typeexpr, writable)

            return None

        elif node.head == 'forstmt':
            self.env.open_scope()
            quan = self.to_ir(node.tail[0])
            if node.tail[1].head == 'stmts':
                stmts = self.to_ir(node.tail[1])
            else:
                stmts = []
            quantifier = (self.env.scope, quan)
            self.env.close_scope()
            return ForStmt(quantifier, stmts, node)

        elif node.head == 'ifstmt':
            ifcond = self.to_ir(node.tail[0])
            if ifcond.result_type is not Bool:
                raise RumurError('expression in conditional does not evaluate to a boolean', node.tail[0])
            if node.tail[1].head == 'stmts':
                ifstmts = self.to_ir(node.tail[1])
                remaining = node.tail[2:]
            else:
                ifstmts = []
                remaining = node.tail[1:]
            branches = [Branch(ifcond, ifstmts, node.tail[0])]
            if isinstance(ifcond, Lit) and ifcond.value:
                # No need to evaluate the parse the rest of the conditional
                return IfStmt(branches, node)
            while remaining[0].head in ('elsif', 'else'):
                n = remaining[0]
                if remaining[0].head == 'elsif':
                    cond = self.to_ir(remaining[1])
                    if cond.result_type is not Bool:
                        raise RumurError('expression in conditional does not evaluate to a boolean', remaining[1])
                    remaining = remaining[2:]
                else:
                    assert remaining[0].head == 'else'
                    cond = None
                    remaining = remaining[1:]
                if remaining[0].head == 'stmts':
                    stmts = self.to_ir(remaining[0])
                    remaining = remaining[1:]
                else:
                    stmts = []
                branches.append(Branch(cond, stmts, n))
            return IfStmt(branches, node)

        elif node.head == 'integer_constant':
            return Lit(int(str(node.tail[0])), node)

        elif node.head == 'invariant':
            if node.tail[0].head == 'string':
                name = self.to_ir(node.tail[0])
            else:
                name = None
            expr = self.to_ir(node.tail[-1])
            if expr.result_type is not Bool:
                raise RumurError('invariant expression does not evaluate to a boolean', node.tail[-1])
            return Invariant(name, expr, node)

        elif node.head == 'proccall':
            symbol = self.to_ir(node.tail[0])
            return ProcCall(symbol, [self.to_ir(x) for x in node.tail[1:]], node)

        elif node.head == 'procedure':
            name = self.to_ir(node.tail[0])
            self.env.open_scope()
            remaining = node.tail[1:]

            # Declare all the parameters
            while remaining[0].head == 'formal':
                self.to_ir(remaining[0])
                remaining = remaining[1:]

            # It's not clear to me whether it is legal for locals to shadow
            # parameters, but we allow them to.
            self.env.open_scope()

            # Declare locals
            while remaining[0].head in ('constdecl', 'typedecl', 'vardecl'):
                self.to_ir(remaining[0])
                remaining = remaining[1:]

            if remaining[0].head == 'stmts':
                stmts = self.to_ir(remaining[0])
            else:
                stmts = []

            decls = self.env.scope
            self.env.close_scope()

            parameters = self.env.scope
            self.env.close_scope()

            return Procedure(name, parameters, decls, stmts, node)

        elif node.head == 'program':
            p = Program(node)
            for t in node.tail:
                x = self.to_ir(t)
                if isinstance(x, Method):
                    p.procs.append(x)
                elif isinstance(x, (AliasRule, Invariant, RuleSet, SimpleRule, StartState)):
                    p.rules.append(x)
            return p

        elif node.head == 'quantifier':
            symbol = self.to_ir(node.tail[0])
            if len(node.tail) == 2:
                typeexpr = self.to_ir(node.tail[1])
                if not isinstance(typeexpr, (TypeEnum, TypeRange)):
                    raise RumurError('type expression in for statement is not a simple type', node.tail[1])
                lower = None
                upper = None
                step = None
                self.env.declare_var(symbol, typeexpr, writable=False)
            else:
                typeexpr = None
                lower = self.to_ir(node.tail[1])
                if not isinstance(lower.result_type, TypeRange):
                    raise RumurError('lower bound in for statement is does not evaluate to an integer', node.tail[1])
                upper = self.to_ir(node.tail[2])
                if not isinstance(upper.result_type, TypeRange):
                    raise RumurError('upper bound in for statement is does not evaluate to an integer', node.tail[1])
                if len(node.tail) == 4:
                    step = self.to_ir(node.tail[3])
                    if not isinstance(step.result_type, TypeRange):
                        raise RumurError('step in for statement is does not evaluate to an integer', node.tail[3])
                    if not isinstance(step, Lit):
                        raise RumurError('step in for statement is not a constant expression', node.tail[3])
                else:
                    assert len(node.tail) == 3
                    step = Lit(1, node)
                self.env.declare_var(symbol, TypeRange(lower, upper, None), writable=False)
            return Quantifier(symbol, typeexpr, lower, upper, step, node)

        elif node.head == 'simplerule':
            remaining = node.tail

            if remaining[0].head == 'string':
                name = self.to_ir(remaining[0])
                remaining = remaining[1:]
            else:
                name = '<unnamed>'

            if remaining[0].head == 'expr':
                guard = self.to_ir(remaining[0])
                remaining = remaining[1:]
            else:
                guard = Lit(True, node)

            self.env.open_scope()
            while remaining[0].head in ('constdecl', 'typedecl', 'vardecl'):
                self.to_ir(remaining[0])
                remaining = remaining[1:]
            decls = self.env.scope
            self.env.close_scope()

            if remaining[0].head == 'stmts':
                stmts = self.to_ir(remaining[0])
            else:
                stmts = []

            return SimpleRule(name, guard, decls, stmts, node)

        elif node.head == 'start':
            return self.to_ir(node.tail[0])

        elif node.head == 'startstate':
            remaining = node.tail

            if remaining[0].head == 'string':
                name = self.to_ir(node.tail[0])
                remaining = remaining[1:]
            else:
                name = '<unnamed>'

            self.env.open_scope()
            while remaining[0].head in ('constdecl', 'typedecl', 'vardecl'):
                self.to_ir(remaining[0])
                remaining = remaining[1:]

            if remaining[0].head == 'stmts':
                stmts = self.to_ir(remaining[0])
            else:
                stmts = []

            decls = self.env.scope
            self.env.close_scope()

            return StartState(name, decls, stmts, node)

        elif node.head == 'stmts':
            stmts = []
            for t in node.tail:
                stmts.append(self.to_ir(t))
            return stmts

        elif node.head == 'string':
            return str(node.tail)[1:-1]

        elif node.head == 'symbol':
            return str(node.tail[0])

        elif node.head == 'typedecl':
            name = self.to_ir(node.tail[0])
            typeexpr = self.to_ir(node.tail[1])
            self.env.declare_type(name, typeexpr)
            return None

        elif node.head == 'typeexpr':

            if node.tail[0].head == 'symbol':
                # Reference to previous type
                name = self.to_ir(node.tail[0])
                try:
                    type = self.env.lookup_type(name)
                except KeyError:
                    raise RumurError('reference to undefined type %s' % name, node)
                return type

            elif node.tail[0].head == 'expr':
                # X..Y
                # We eagerly constant fold the bounds to allow use of more complicated expressions.
                lower = constant_fold(self.to_ir(node.tail[0]))
                if not isinstance(lower, Lit):
                    raise RumurError('lower bound of range type is not constant', node)
                upper = constant_fold(self.to_ir(node.tail[1]))
                if not isinstance(upper, Lit):
                    raise RumurError('upper bound of range type is not constant', node)
                return TypeRange(lower.value, upper.value, node)

            elif node.tail[0].head == 'array':
                # array[X] of Y
                index_type = self.to_ir(node.tail[1])
                if not isinstance(index_type, (TypeEnum, TypeRange)):
                    raise RumurError('index type of array is not a simple type', node)
                member_type = self.to_ir(node.tail[2])
                return TypeArray(index_type, member_type, node)

        elif node.head == 'vardecl':
            typeexpr = self.to_ir(node.tail[-1])
            for symbol in node.tail[:-1]:
                name = self.to_ir(symbol)
                self.env.declare_var(name, typeexpr, writable=True)
            return None

        raise NotImplementedError('%d: no IR equivalent of AST node %s (in `%s`)' % (lineno(node), node.head, node))

def to_ir(node):
    g = Generator()
    ir = g.to_ir(node)
    assert len(g.env.scopes) == 1
    return g.env.scope, ir
