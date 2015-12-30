from ConstantFolding import constant_fold
from Environment import Environment
from IR import AliasRule, Invariant, Lit, Method, Program, RuleSet, SimpleRule, StartState, TypeDecl, VarDecl
from RumurError import RumurError

class Generator(object):

    def __init__(self):
        self.env = Environment()

    def to_ir(self, node):

        if node.head == 'constdecl':
            name = str(node.tail[0])
            expr = self.to_ir(node.tail[1])
            # Any constant declaration should be able to be folded immediately.
            expr = constant_fold(self.env, expr)
            if not isinstance(expr, Lit):
                raise RumurError('%s:%d: constant declaration is not constant' %
                    node.filename, node.lineno)
            self.env.declare(name, expr)
            return None

        elif node.head == 'program':
            p = Program()
            for t in node.tail:
                x = self.to_ir(t)
                if isinstance(x, (TypeDecl, VarDecl)):
                    p.decls.append(x)
                elif isinstance(x, Method):
                    p.procs.append(x)
                elif isinstance(x, (AliasRule, Invariant, RuleSet, SimpleRule, StartState)):
                    p.rules.append(x)
            return p

        elif node.head == 'start':
            return self.to_ir(node.tail[0])

        raise NotImplementedError('no IR equivalent of AST node %s (in `%s`)' % (node.head, node))

def to_ir(node):
    g = Generator()
    return g.to_ir(node)
