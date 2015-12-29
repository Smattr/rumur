from Environment import Environment
from IR import AliasRule, ConstDecl, Invariant, Method, Program, RuleSet, SimpleRule, StartState, TypeDecl, VarDecl

class Generator(object):

    def __init__(self):
        self.env = Environment()

    def to_ir(self, node):

        if node.head == 'program':
            p = Program()
            for t in node.tail:
                x = self.to_ir(t)
                if isinstance(x, (ConstDecl, TypeDecl, VarDecl)):
                    p.decls.append(x)
                elif isinstance(x, Method):
                    p.procs.append(x)
                else:
                    assert isinstance(x, (AliasRule, Invariant, RuleSet, SimpleRule, StartState))
                    p.rules.append(x)
            return p

        elif node.head == 'start':
            return self.to_ir(node.tail[0])

        raise NotImplementedError('no IR equivalent of AST node %s (in `%s`)' % (node.head, node))

def to_ir(node):
    g = Generator()
    return g.to_ir(node)
