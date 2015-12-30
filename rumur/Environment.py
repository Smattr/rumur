from Access import StateSymbol, DirectSymbol
from Type import Constant

class Environment(object):
    def __init__(self):
        self.scopes = []
        self.global_scope_offset = 1
        # The state begins inaccessible and remains inaccessible in the first
        # scope (the global scope). It is only accessible in child scopes when
        # it is passed in as a parameter and then needs to be registered below.
        self.state_variable = None

    def register_state(self, symbol):
        assert self.state_variable is None, \
            'registering the state as \'%s\' while it is already accessible ' \
            'as \'%s\'' % (symbol, self.state_variable)
        self.state_variable = symbol

    def deregister_state(self):
        assert self.state_variable is not None, \
            'deregistering the state when it is already inaccessible'
        self.state_variable = None

    def new_scope(self):
        self.scopes.append({})

    def pop_scope(self):
        assert len(self.scopes) > 1, 'attempt to pop the global scope'
        self.scopes.pop()

    def declare(self, symbol, type):
        assert len(self.scopes) > 0, 'declaring variables before opening the global scope'
        if symbol in self.scopes[-1]:
            raise Exception('%s already declared in this scope' % symbol)
        if len(self.scopes) == 1:
            if isinstance(type, Constant):
                # XXX
            # We're declaring a new global (i.e. part of the state)
            s = StateSymbol(symbol, type, self.global_scope_offset)
            self.global_scope_offset *= type.cardinality()
        else:
            s = DirectSymbol(symbol, type)
        self.scopes[-1][symbol] = s

    def typedef(self, symbol, type):
        assert len(self.scopes) > 0, 'declaring types before opening the global scope'
        if symbol in self.scopes[-1]:
            raise Exception('%s already declared in this scope' % symbol)
        self.scopes[-1][symbol] = type

    def lookup(self, symbol):
        for scope in reversed(self.scopes):
            if symbol in scope:
                return scope[symbol]
        return None

    def bare(self):
        '''Whether we're in the global scope (outside any procedure, rules,
        etc.'''
        return len(self.scopes) == 1
