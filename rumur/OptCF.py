class ConstantFolder(object):

    def __init__(self, env):
        self.env = env

    def visit(self, node):

        return node

def constant_fold(env, ir):
    cf = ConstantFolder(env)
    return ir.preorder(cf.visit)
