from Traversal import PreorderTraversal

class ConstantFolder(PreorderTraversal):

    def visitor(self, node):

        if node.head == 'expr':

            if is_binary_op(node):
                left = op1(node)
                right = op2(node)

            if len(node.tail) > 1 and node.tail[1].head == 'and':

                if is_literal(left):
                    if literal_value(left):
                        return left
                    return right

                elif is_literal(right):
                    if literal_value(right):
                        return right
                    return left

            elif len(node.tail) > 1 and node.tail[1].head == 'or':

                if is_literal(left):
                    if literal_value(left):
                        return right
                    return left

                elif is_literal(right):
                    if literal_value(right):
                        return left
                    return right

            elif node.tail[0].head == 'not':

                if node.tail[1].tail[0].head == 'not':
                    # !!X ==> X
                    return node.tail[1].tail[1]

            elif len(node.tail) > 1 and node.tail[1].head == 'question':
                a = node.tail[0]
                b = node.tail[2]
                c = node.tail[4]

                if is_literal(a):
                    if literal_value(a):
                        return b
                    return c

                if b == c:
                    return b

        return node


def constant_fold(ast):
    cf = ConstantFolder()
    a = cf.visit(ast)
    return cf.modified, a

def is_binary_op(node):
    return node.head == 'expr' and len(node.tail) == 3 and node.tail[1].head \
        in ('add', 'sub', 'mul', 'div', 'mod', 'or', 'and', 'implies', 'lt',
        'lte', 'gt', 'gte', 'eq', 'neq')

def op1(node):
    assert is_binary_op(node)
    return node.tail[0]

def op2(node):
    assert is_binary_op(node)
    return node.tail[2]

def is_literal(node):
    return node.head == 'expr' and len(node.tail) > 0 and \
        node.tail[0].head == 'integer_constant'

def literal_value(node):
    assert is_literal(node)
    return int(str(node.tail[0]))
