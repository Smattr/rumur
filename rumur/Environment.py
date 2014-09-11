from Access import StateSymbol, DirectSymbol

class Environment(object):
    def __init__(self):
        self.scopes = []
        self.global_scope_offset = 1

    def new_scope(self):
        self.scopes.append({})

    def pop_scope(self):
        assert len(self.scopes) > 1, 'attempt to pop the global scope'
        self.scopes.pop()

    def declare(self, symbol, type, via_state):
        assert len(self.scopes) > 0, 'declaring variables before opening the global scope'
        if symbol in self.scopes[-1]:
            raise Exception('%s already declared in this scope' % symbol)
        if (via_state is None and len(self.scopes) == 1) or via_state:
            # We're declaring a new global (i.e. part of the state)
            s = StateSymbol(symbol, type, self.global_scope_offset)
            self.global_scope_offset *= type.cardinality()
        else:
            s = DirectSymbol(symbol, type)
        self.scopes[-1][symbol] = s

    def lookup(self, expr):
        raise NotImplementedError

    def typeof(self, expr):
        data = self.lookup(expr)
        if data is None:
            return None
        return data.type

    def access(self, expr):
        data = self.lookup(expr)
        if data is None:
            return None
        return data.access
