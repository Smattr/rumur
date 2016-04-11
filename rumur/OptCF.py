'''
Optimisation - Constant folding
'''

from IR import Add, And, BoolEq, BoolNEq, Div, GT, GTE, Imp, IntEq, IntNEq, Lit, LT, LTE, Mod, \
    Mul, Not, Or, Sub, TriCond
from RumurError import RumurError

# XXX: Clagged from IRGenerator
def lineno(stree):
    stree.calc_position()
    return stree.min_line

def _fold(e):

    if isinstance(e, Add) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        return Lit(e.left + e.right, e.node)

    if isinstance(e, Sub) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        return Lit(e.left - e.right, e.node)

    if isinstance(e, Mul) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        return Lit(e.left * e.right, e.node)

    if isinstance(e, Div) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        if e.right.value == 0:
            raise RumurError('%d: division by zero' % lineno(e.node))
        return Lit(e.left // e.right, e.node)

    if isinstance(e, Mod) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        if e.right.value == 0:
            raise RumurError('%d: modulo by zero' % lineno(e.node))
        return Lit(e.left % e.right, e.node)

    if isinstance(e, Or) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        return Lit(e.left or e.right, e.node)

    if isinstance(e, And) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        return Lit(e.left and e.right, e.node)

    if isinstance(e, Imp) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        return Lit(not e.left or e.right, e.node)

    if isinstance(e, LT) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        return Lit(e.left < e.right, e.node)

    if isinstance(e, LTE) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        return Lit(e.left <= e.right, e.node)

    if isinstance(e, GT) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        return Lit(e.left > e.right, e.node)

    if isinstance(e, GTE) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        return Lit(e.left >= e.right, e.node)

    if isinstance(e, (BoolEq, IntEq)) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        return Lit(e.left == e.right, e.node)

    if isinstance(e, (BoolNEq, IntNEq)) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        return Lit(e.left != e.right, e.node)

    if isinstance(e, Not) and isinstance(e.operand, Lit):
        return Lit(not e.operand, e.node)

    if isinstance(e, TriCond) and isinstance(e.cond, Lit):
        if e.cond.value:
            return e.casetrue
        return e.casefalse

    return expr

def constant_fold(ast):
    return ast.postorder(_fold)
