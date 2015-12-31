from ConstantFolding import constant_fold
from Environment2 import Environment
from IR import Add, AliasRule, And, Assignment, Branch, Eq, GT, IfStmt, Invariant, Lit, LT, Method, Not, ProcCall, Procedure, Program, RuleSet, SimpleRule, StartState, TypeArray, TypeEnum, TypeRange, VarRead, VarWrite
from RumurError import RumurError

def lineno(stree):
    stree.calc_position()
    return stree.min_line

class Generator(object):

    def __init__(self):
        self.env = Environment()

    def to_ir(self, node, lvalue=False):

        if node.head == 'assignment':
            designator = self.to_ir(node.tail[0], lvalue=True)
            expr = self.to_ir(node.tail[1])
            return Assignment(designator, expr, node)

        elif node.head == 'constdecl':
            name = str(node.tail[0].tail[0])
            expr = self.to_ir(node.tail[1])
            if not isinstance(expr, Lit):
                raise RumurError('%d: constant declaration is not constant' %
                    lineno(node))
            self.env.declare_constant(name, expr.value)
            return None

        elif node.head == 'designator':
            root = str(node.tail[0].tail[0])
            try:
                value = self.env.lookup_var(root)
            except KeyError:
                raise RumurError('%d: designator %s refers to an '
                    'undefined variable' % (lineno(node),
                    root))
            if isinstance(value, (int, bool)):
                # This reference is to a constant
                if len(node.tail) > 1:
                    raise RumurError('%d: excess qualifiers after '
                        'reference to a constant' % lineno(node))
                if lvalue:
                    raise RumurError('%d: attempted assignment to a '
                        'constant' % lineno(node))
                return Lit(value, node)
            stems = []
            result_type = value[0]
            for t in node.tail[1:]:
                if t.head == 'symbol':
                    sym = str(t.tail[0])
                    if not isinstance(result_type, TypeRecord):
                        raise RumurError('%d: %s is not a member of the '
                            'preceding expression' % (lineno(t), sym))
                    try:
                        member = result_type.lookup_var(sym)
                    except KeyError:
                        raise RumurError('%d: %s is not a member of the '
                            'preceding expression' % (lineno(t), sym))
                    result_type = member[0]
                    stems.append(sym)
                else:
                    assert t.head == 'expr'
                    if not isinstance(result_type, TypeArray):
                        raise RumurError('%d: array index on a type that is '
                            'not an array' % lineno(t))
                    result_type = result_type.member_type
                    stems.append(self.to_ir(t))
            if lvalue:
                if not value[1]:
                    raise RumurError('%d: attempt to write to read-only '
                        'variable' % lineno(node))
                return VarWrite(root, stems, node)
            return VarRead(root, stems, result_type, node)


        # When handling expressions in the following, we do some
        # simplification and constant folding where obvious.

        elif node.head == 'expr':
            if node.tail[0].head == 'expr1':
                return self.to_ir(node.tail[0])
            raise NotImplementedError

        elif node.head == 'expr1':
            if node.tail[0].head == 'expr2':
                return self.to_ir(node.tail[0])
            raise NotImplementedError

        elif node.head == 'expr2':
            if node.tail[0].head == 'expr3':
                return self.to_ir(node.tail[0])
            raise NotImplementedError

        elif node.head == 'expr3':
            if node.tail[0].head == 'expr4':
                return self.to_ir(node.tail[0])

            assert node.tail[1].head == 'and'
            left = self.to_ir(node.tail[0])
            if left.result_type is not bool:
                raise RumurError('%d: left operand to and does '
                    'not evaluate to a boolean' % lineno(node))
            right = self.to_ir(node.tail[2])
            if right.result_type is not bool:
                raise RumurError('%d: right operand to and does '
                    'not evaluate to a boolean' % lineno(node))
            if isinstance(left, Lit):
                if left.value:
                    return right
                return Lit(False, node)
            if isinstance(right, Lit):
                if right.value:
                    return left
                return Lit(False, node)
            return And(left, right, node)

        elif node.head == 'expr4':
            if node.tail[0].head == 'expr5':
                return self.to_ir(node.tail[0])

            assert node.tail[0].head == 'not'
            operand = self.to_ir(node.tail[1])
            if operand.result_type is not bool:
                raise RumurError('%d: operand to not does '
                    'not evaluate to a boolean' % lineno(node))
            if isinstance(operand, Lit):
                if operand.value:
                    return Lit(False, node)
                return Lit(True, node)
            if isinstance(operand, Not):
                # !!X ==> X
                return operand.operand
            return Not(operand, node)

        elif node.head == 'expr5':
            if node.tail[0].head == 'expr6':
                return self.to_ir(node.tail[0])

            left = self.to_ir(node.tail[0])
            right = self.to_ir(node.tail[2])

            if node.tail[1].head == 'lt':
                if left.result_type is not int:
                    raise RumurError('%d: left operand to less than does '
                        'not evaluate to an integer' % lineno(node))
                if right.result_type is not int:
                    raise RumurError('%d: right operand to less than does '
                        'not evaluate to an integer' % lineno(node))
                if isinstance(left, Lit) and isinstance(right, Lit):
                    return Lit(left.value < right.value, node)
                return LT(left, right, node)

            elif node.tail[1].head == 'eq':
                if left.result_type is not right.result_type:
                    raise RumurError('%d: equality comparison between '
                        'expressions of differing types' % lineno(node))
                if isinstance(left, Lit) and isinstance(right, Lit):
                    return Lit(left.value == right.value, node)
                if isinstance(left, Lit) and left.result_type is bool:
                    if left.value:
                        return right
                    return Not(right, node)
                if isinstance(right, Lit) and right.result_type is bool:
                    if right.value:
                        return left
                    return Not(left, node)
                return Eq(left, right, node)

            elif node.tail[1].head == 'gt':
                if left.result_type is not int:
                    raise RumurError('%d: left operand to greater than does '
                        'not evaluate to an integer' % lineno(node))
                if right.result_type is not int:
                    raise RumurError('%d: right operand to greater than does '
                        'not evaluate to an integer' % lineno(node))
                if isinstance(left, Lit) and isinstance(right, Lit):
                    return Lit(left.value > right.value, node)
                return GT(left, right, node)

        elif node.head == 'expr6':
            if node.tail[0].head == 'expr7':
                return self.to_ir(node.tail[0])

            if node.tail[1].head == 'add':
                left = self.to_ir(node.tail[0])
                if left.result_type is not int:
                    raise RumurError('%d: left operand to addition does '
                        'not evaluate to an integer' % lineno(node))
                right = self.to_ir(node.tail[2])
                if right.result_type is not int:
                    raise RumurError('%d: right operand to addition does '
                        'not evaluate to an integer' % lineno(node))
                if isinstance(left, Lit) and isinstance(right, Lit):
                    return Lit(left.value + right.value, node)
                if isinstance(left, Lit) and left.value == 0:
                    return right
                if isinstance(right, Lit) and right.value == 0:
                    return left
                return Add(left, right, node)

            assert node.tail[1].head == 'sub'
            left = self.to_ir(node.tail[0])
            if left.result_type is not int:
                raise RumurError('%d: left operand to subtraction does '
                    'not evaluate to an integer' % lineno(node))
            right = self.to_ir(node.tail[2])
            if right.result_type is not int:
                raise RumurError('%d: right operand to subtraction does '
                    'not evaluate to an integer' % lineno(node))
            if isinstance(left, Lit) and isinstance(right, Lit):
                return Lit(left.value - right.value, node)
            if isinstance(right, Lit) and right.value == 0:
                return left
            return Sub(left, right, node)

        elif node.head == 'expr7':
            if node.tail[0].head == 'expr8':
                return self.to_ir(node.tail[0])

            if node.tail[1].head == 'mul':
                left = self.to_ir(node.tail[0])
                if left.result_type is not int:
                    raise RumurError('%d: left operand to multiplication does '
                        'not evaluate to an integer' % lineno(node))
                right = self.to_ir(node.tail[2])
                if right.result_type is not int:
                    raise RumurError('%d: right operand to multiplication does '
                        'not evaluate to an integer' % lineno(node))
                if isinstance(left, Lit) and isinstance(right, Lit):
                    return Lit(left.value * right.value, node)
                if isinstance(left, Lit) and left.value == 1:
                    return right
                if isinstance(right, Lit) and right.value == 1:
                    return left
                return Mul(left, right, node)

            elif node.tail[1].head == 'div':
                left = self.to_ir(node.tail[0])
                if left.result_type is not int:
                    raise RumurError('%d: left operand to division does '
                        'not evaluate to an integer' % lineno(node))
                right = self.to_ir(node.tail[2])
                if right.result_type is not int:
                    raise RumurError('%d: right operand to division does '
                        'not evaluate to an integer' % lineno(node))
                if isinstance(right, Lit) and right.value == 0:
                    # This is a division by zero, which is a runtime error.
                    # Optimise the numerator away entirely.
                    return Div(Lit(0, node), right, node)
                if isinstance(left, Lit) and isinstance(right, Lit):
                    assert right.value != 0
                    return Lit(left.value / right.value, node)
                if isinstance(right, Lit) and right.value == 1:
                    return left
                return Div(left, right, node)

        elif node.head == 'expr8':

            if node.tail[0].head == 'designator':
                return self.to_ir(node.tail[0], lvalue=False)

            elif node.tail[0].head == 'integer_constant':
                return Lit(int(str(node.tail[0].tail[0])), node)

        elif node.head == 'formal':
            if node.tail[0].head == 'var':
                writable = True
                remaining = node.tail[1:]
            else:
                writable = False
                remaining = node.tail

            typeexpr = self.to_ir(remaining[-1])

            for symbol in remaining[:-1]:
                name = str(symbol.tail[0])
                self.env.declare_var(name, typeexpr, writable)

            return None

        elif node.head == 'ifstmt':
            ifcond = self.to_ir(node.tail[0])
            if ifcond.result_type is not bool:
                raise RumurError('%d: expression in conditional does not '
                    'evaluate to a boolean' % lineno(node.tail[0]))
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
                    if cond.result_type is not bool:
                        raise RumurError('%d: expression in conditional does not '
                            'evaluate to a boolean' % lineno(remaining[1]))
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
                # Only append the conditional if it is not always-false
                if not isinstance(cond, Lit) or not cond.value:
                    branches.append(Branch(cond, stmts, n))
                if isinstance(cond, Lit) and cond.value:
                    # No need to parse the rest
                    break
            return IfStmt(branches, node)

        elif node.head == 'proccall':
            symbol = str(node.tail[0].tail)
            return ProcCall(symbol, [self.to_ir(x) for x in node.tail[1:]], node)

        elif node.head == 'procedure':
            name = str(node.tail[0].tail[0])
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

        elif node.head == 'stmts':
            stmts = []
            for t in node.tail:
                stmts.append(self.to_ir(t))
            return stmts

        elif node.head == 'string':
            return str(node.tail)[1:-1]

        elif node.head == 'typedecl':
            name = str(node.tail[0].tail[0])
            typeexpr = self.to_ir(node.tail[1])
            self.env.declare_type(name, typeexpr)
            return None

        elif node.head == 'typeexpr':

            if node.tail[0].head == 'symbol':
                # Reference to previous type
                name = str(node.tail[0].tail[0])
                try:
                    type = self.env.lookup_type(name)
                except KeyError:
                    raise RumurError('%d: reference to undefined type %s' %
                        (lineno(node), name))
                return type

            elif node.tail[0].head == 'expr':
                # X..Y
                lower = self.to_ir(node.tail[0])
                if not isinstance(lower, Lit):
                    raise RumurError('%d: lower bound of range type is not '
                        'constant' % lineno(node))
                upper = self.to_ir(node.tail[1])
                if not isinstance(upper, Lit):
                    raise RumurError('%d: upper bound of range type is not '
                        'constant' % lineno(node))
                return TypeRange(lower, upper, node)

            elif node.tail[0].head == 'array':
                # array[X] of Y
                index_type = self.to_ir(node.tail[1])
                if not isinstance(index_type, (TypeEnum, TypeRange)):
                    raise RumurError('%d: index type of array is not a simple '
                        'type' % lineno(node))
                member_type = self.to_ir(node.tail[2])
                return TypeArray(index_type, member_type, node)

        elif node.head == 'vardecl':
            typeexpr = self.to_ir(node.tail[-1])
            for symbol in node.tail[:-1]:
                name = str(symbol.tail[0])
                self.env.declare_var(name, typeexpr, writable=True)
            return None

        raise NotImplementedError('%d: no IR equivalent of AST node %s (in `%s`)' % (lineno(node), node.head, node))

def to_ir(node):
    g = Generator()
    return g.to_ir(node)
