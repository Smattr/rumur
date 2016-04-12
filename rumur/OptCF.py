'''
Optimisation - Constant folding
'''

from IR import Add, And, BoolEq, BoolNEq, Div, GT, GTE, Imp, IntEq, IntNEq, Lit, LT, LTE, Mod, \
    Mul, Not, Or, Sub, TriCond
from Log import log
from RumurError import RumurError

def _fold(e):

    if isinstance(e, Add) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        log.debug('folding Add of two literals')
        return Lit(e.left.value + e.right.value, e.node)

    if isinstance(e, Add) and isinstance(e.left, Lit) and e.left.value == 0:
        log.debug('folding Add of 0')
        return e.right

    if isinstance(e, Add) and isinstance(e.right, Lit) and e.right.value == 0:
        log.debug('folding Add of 0')
        return e.left

    if isinstance(e, Sub) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        log.debug('folding Sub of two literals')
        return Lit(e.left.value - e.right.value, e.node)

    if isinstance(e, Sub) and isinstance(e.right, Lit) and e.right.value == 0:
        log.debug('folding Sub of 0')
        return e.left

    if isinstance(e, Mul) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        log.debug('folding Mul of two literals')
        return Lit(e.left.value * e.right.value, e.node)

    if isinstance(e, Mul) and isinstance(e.left, Lit) and e.left.value == 1:
        log.debug('folding Mul of 1')
        return e.right

    if isinstance(e, Mul) and isinstance(e.right, Lit) and e.right.value == 1:
        log.debug('folding Mul of 1')
        return e.left

    if isinstance(e, Div) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        log.debug('folding Div of two literals')
        if e.right.value == 0:
            raise RumurError('division by zero', e.node)
        return Lit(e.left.value // e.right.value, e.node)

    if isinstance(e, Div) and isinstance(e.right, Lit) and e.right.value == 1:
        log.debug('folding Div by 1')
        return e.left

    if isinstance(e, Mod) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        log.debug('folding Mod of two literals')
        if e.right.value == 0:
            raise RumurError('modulo by zero', e.node)
        return Lit(e.left.value % e.right.value, e.node)

    if isinstance(e, Mod) and isinstance(e.right, Lit) and e.right.value == 1:
        log.debug('folding Mod by 1')
        return Lit(0, e.node)

    if isinstance(e, Or) and isinstance(e.left, Lit):
        log.debug('folding Or of literal')
        if e.left.value:
            return Lit(True, e.node)
        return e.right

    if isinstance(e, Or) and isinstance(e.right, Lit):
        log.debug('folding Or of literal')
        if e.right.value:
            return Lit(True, e.node)
        return e.left

    if isinstance(e, And) and isinstance(e.left, Lit):
        log.debug('folding And of literal')
        if e.left.value:
            return e.right
        return Lit(False, e.node)

    if isinstance(e, And) and isinstance(e.right, Lit):
        log.debug('folding And of literal')
        if e.right.value:
            return e.left
        return Lit(False, e.node)

    if isinstance(e, LT) and isinstance(e.left, Lit) and isinstance(e.right, Lit):
        log.debug('folding LT of two literals')
        return Lit(e.left.value < e.right.value, e.node)

    if isinstance(e, Not) and isinstance(e.operand, Lit):
        log.debug('folding Not of literal')
        return Lit(not e.operand, e.node)

    if isinstance(e, TriCond) and isinstance(e.cond, Lit):
        log.debug('folding TriCond of literal')
        if e.cond.value:
            return e.casetrue
        return e.casefalse

    return e

def constant_fold(ast):
    return ast.postorder(_fold)
