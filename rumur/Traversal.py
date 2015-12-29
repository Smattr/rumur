import abc, six

class Traversal(six.with_metaclass(abc.ABCMeta, object)):

    def __init__(self):
        self.modified = False

    @abc.abstractmethod
    def visit(self, node):
        raise NotImplementedError

    @abc.abstractmethod
    def visitor(self, node):
        raise NotImplementedError

    def _visit_children(self, node):
        if not hasattr(node, 'tail'):
            return
        i = 0
        while i < len(node.tail):
            if hasattr(node.tail[i], 'head'):
                replacement = self.visit(node.tail[i])
                if replacement is None:
                    self.modified = True
                    node.tail = node.tail[:i] + node.tail[i+1:]
                    continue
                elif replacement is not node.tail[i]:
                    self.modified = True
                    node.tail[i] = replacement
            i += 1

class PreorderTraversal(six.with_metaclass(abc.ABCMeta, Traversal)):

    def visit(self, node):
        self._visit_children(node)
        return self.visitor(node)

class PostorderTraversal(six.with_metaclass(abc.ABCMeta, Traversal)):

    def visit(self, node):
        n = self.visitor(node)
        self._visit_children(n)
        return n
