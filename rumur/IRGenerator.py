from ConstantFolding import constant_fold
from Environment2 import Environment
from IR import AliasRule, And, Invariant, Lit, Method, Not, Program, RuleSet, SimpleRule, StartState, TypeArray, TypeEnum, TypeRange, VarDecl
from RumurError import RumurError

def lineno(stree):
    stree.calc_position()
    return stree.min_line

class Generator(object):

    def __init__(self):
        self.env = Environment()

    def to_ir(self, node, lvalue=False):

        if node.head == 'constdecl':
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
            raise NotImplementedError


        elif node.head == 'expr':

            # When handling expressions in the following, we do some
            # simplification and constant folding where obvious.

            if node.tail[0].head == 'designator':
                return self.to_ir(node.tail[0], lvalue=False)

            elif node.tail[0].head == 'integer_constant':
                return Lit(int(str(node.tail[0].tail[0])), node)

            elif node.tail[1].head == 'add':
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

            elif node.tail[1].head == 'sub':
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

            elif node.tail[1].head == 'mul':
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

            elif node.tail[1].head == 'and':
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

            elif node.tail[0].head == 'not':
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

            elif node.tail[1].head == 'eq':
                left = self.to_ir(node.tail[0])
                right = self.to_ir(node.tail[2])
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

        elif node.head == 'program':
            p = Program(node)
            for t in node.tail:
                x = self.to_ir(t)
                if isinstance(x, Method):
                    p.procs.append(x)
                elif isinstance(x, (AliasRule, Invariant, RuleSet, SimpleRule, StartState)):
                    p.rules.append(x)
            return p

        elif node.head == 'start':
            return self.to_ir(node.tail[0])

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

        raise NotImplementedError('no IR equivalent of AST node %s (in `%s`)' % (node.head, node))

def to_ir(node):
    g = Generator()
    return g.to_ir(node)
