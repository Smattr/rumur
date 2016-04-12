'''
Optimisation - Strength reduction

This is not so much an optimisation as a simplification. It allows passes that run after this to
omit cases to deal with non-core IR entities.
'''

from IR import And, Assignment, Bool, BoolEq, BoolNEq, Branch, ClearStmt, Designator, ErrorStmt, \
    Exists, Forall, ForStmt, GT, GTE, IfStmt, Imp, IntEq, IntNEq, Invariant, Lit, LT, LTE, Not, \
    Or, Quantifier, SimpleRule, SwitchStmt, TypeArray, TypeEnum, TypeRange, TypeRecord
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

    if isinstance(n, ClearStmt):
        log.debug('simplifying ClearStmt')

        def clear(d, counter):
            t = d.result_type
            if t is Bool:
                return Assignment(d, Lit(False, n.node), n.node)
            elif isinstance(t, TypeRange):
                return Assignment(d, Lit(t.lower, n.node), n.node)
            elif isinstance(t, TypeEnum):
                return Assignment(d, Designator(False, t.members[0], [], t, n.node), n.node)
            elif isinstance(t, TypeRecord):
                stmts = []
                for field, var in t.vardecls.vars.items():
                    child = Designator(d.in_state, d.root, d.stems + [field], var.typ, n.node)
                    stmts.append(clear(child, counter))
                return IfStmt([Branch(Lit(True, n.node), stmts, n.node)], n.node)
            else:
                assert isinstance(t, TypeArray)
                loop_var_name = '_%d' % counter
                counter += 1
                loop_var_ref = Designator(False, loop_var_name, [], t.index_type, n.node)
                child = Designator(d.in_state, d.root, d.stems + [loop_var_ref], t.member_type, n.node)
                stmt = clear(child, counter)
                quan = Quantifier(loop_var_name, t.index_type, None, None, None, n.node)
                return ForStmt(quan, [stmt], n.node)

        return clear(n.designator, 1)

    return n

def strength_reduce(ast):
    return ast.postorder(_reduce)
