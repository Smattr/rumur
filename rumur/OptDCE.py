'''
Optimisation - Dead code elimination
'''

from IR import ForStmt, IfStmt, Lit, Node, PutStmt, WhileStmt
from Log import log
from RumurError import RumurError

def is_tautology(expr):
    if isinstance(expr, Lit) and expr.value is True:
        return True
    # TODO
    return False

def is_contradiction(expr):
    if isinstance(expr, Lit) and expr.value is False:
        return True
    # TODO
    return False

class _NullStatement(Node):
    '''
    Artificial node that we'll insert as a sentinel then clean up in a final pass.
    '''
    def __init__(self):
        pass
NullStatement = _NullStatement()

def _dce(n):

    # Purge all null statements that are list-hosted children of this one. Note that because this
    # pass runs post-order, children have already been dealt with.
    for k, v in n.__dict__.items():
        if isinstance(v, (tuple, list)):
            vs = [s for s in v if s is not NullStatement]
            if isinstance(v, tuple):
                vs = tuple(vs)
            if len(vs) != len(v):
                setattr(n, k, vs)

    if isinstance(n, IfStmt):

        # Prune branches that will never be hit.
        index = 0
        while index < len(n.branches):
            b = n.branches[index]
            if index < len(n.branches) - 1 and is_tautology(b.cond):
                # All remaining branches are dead code.
                log.debug('pruning branches from IfStmt')
                n.branches = n.branches[:index+1]
                break
            elif is_contradiction(b.cond):
                # This branch is dead.
                log.debug('pruning branch from IfStmt')
                n.branches = n.branches[:index] + n.branches[index+1:]
            else:
                index += 1

        # Prune trailing branches that contain no statements.
        index = len(n.branches) - 1
        while index >= 0 and len(n.branches[index].stmts) == 0:
            log.debug('pruning trailing branch from IfStmt')
            n.branches = n.branches[:index]
            index -= 1

        if len(n.branches) == 0:
            log.debug('eliminating IfStmt')
            return NullStatement

    if isinstance(n, WhileStmt):
        if is_tautology(n.expr):
            raise RumurError('infinite loop', n.node)

        if is_contradiction(n.expr) or len(n.stmts) == 0:
            log.debug('eliminating WhileStmt')
            return NullStatement

    if isinstance(n, PutStmt) and n.arg == '':
        log.debug('eliminating PutStmt')
        return NullStatement

    if isinstance(n, ForStmt):
        if len(n.stmts) == 0:
            log.debug('eliminating ForStmt')
            return NullStatement

    return n

def eliminate_dead_code(ast):
    return ast.postorder(_dce)
