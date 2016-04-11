'''
Optimisation - Strength reduction

This is not so much an optimisation as a simplification. It allows passes that run after this to
omit cases to deal with non-core IR entities.
'''

from IR import And, BoolEq, BoolNEq, Branch, ErrorStmt, Exists, Forall, GT, GTE, IfStmt, Imp, \
    IntEq, IntNEq, Invariant, LT, LTE, Not, Or, SimpleRule, SwitchStmt
from Log import log

def _reduce(n):

    if isinstance(n, Imp):
        log.debug('simplifying implies')
        return Or(Not(n.left, n.node), n.right, n.node)

    if isinstance(n, LTE):
        log.debug('simplifying lte')
        return Or(LT(n.left, n.right, n.node), IntEq(n.left, n.right, n.node), n.node)

    if isinstance(n, GT):
        log.debug('simplifying gt')
        return LT(n.right, n.left, n.node)

    if isinstance(n, GTE):
        log.debug('simplifying gte')
        return Or(LT(n.right, n.left, n.node), IntEq(n.left, n.right, n.node), n.node)

    if isinstance(n, BoolEq):
        log.debug('simplifying booleq')
        return Or(And(n.left, n.right, n.node),
                  And(Not(n.left, n.node), Not(n.right, n.node), n.node), n.node)

    if isinstance(n, BoolNEq):
        log.debug('simplifying boolneq')
        return Or(And(n.left, Not(n.right, n.node), n.node),
                  And(Not(n.left, n.node), n.right, n.node), n.node)

    if isinstance(n, IntNEq):
        log.debug('simplifying intneq')
        return Not(IntEq(n.left, n.right, n.node), n.node)

    if isinstance(n, Exists):
        log.debug('simplifying exists')
        return Not(Forall(n.quantifier, Not(n.expr, n.node), n.node), n.node)

    if isinstance(n, SwitchStmt):
        log.debug('simplifying switch')
        branches = []
        for c in n.cases:
            if n.expr.result_type == bool:
                cond = Or(And(n.expr, c.expr, n.node),
                          And(Not(n.expr, n.node), Not(c.expr, n.node), n.node), n.node)
            else:
                cond = IntEq(n.expr, c.expr, n.node)
            b = Branch(cond, c.stmts, n.node)
            branches.append(b)
        return IfStmt(branches, n.node)

    if isinstance(n, Invariant):
        log.debug('simplifying invariant')
        return SimpleRule(n.string, Not(n.expr, n.node), [],
            [ErrorStmt('Invariant violated: %s' % n.string, n.node)], n.node)

    return n

def strength_reduce(ast):
    return ast.postorder(_reduce)
