from IR import Lit, TypeEnum
from Log import log
from RumurError import RumurError

class Scope(object):

    def __init__(self):
        self.constants = {}
        self.types = {}
        self.vars = {}

    def declare_constant(self, key, value):
        if key in self.constants:
            raise RumurError('%s conflicts with a constant declared in the '
                'same scope' % key)
        if key in self.vars:
            raise RumurError('%s conflicts with a variable declared in the '
                'same scope' % key)
        self.constants[key] = value

    def declare_var(self, key, type):
        if key in self.constants:
            raise RumurError('%s conflicts with a constant declared in the '
                'same scope' % key)
        if key in self.vars:
            raise RumurError('%s conflicts with a variable declared in the '
                'same scope' % key)
        self.vars[key] = type

    def lookup_var(self, key):
        try:
            return self.constants[key]
        except KeyError:
            return self.vars[key]

    def declare_type(self, key, type):
        if key in self.types:
            raise RumurError('%s conflicts with a type declared in the same '
                'scope' % key)
        self.types[key] = type

    def lookup_type(self, key):
        return self.types[key]

class Environment(object):

    def __init__(self):
        self.scopes = []
        self.open_scope()
        self.declare_type('boolean', TypeEnum(('false', 'true'), None))
        self.declare_constant('true', True)
        self.declare_constant('false', False)


    def open_scope(self):
        self.scopes.append(Scope())

    def close_scope(self):
        self.scopes.pop()

    def declare_constant(self, key, value):
        assert len(self.scopes) >= 1
        log.debug('declaring constant %s' % key)
        self.scopes[-1].declare_constant(key, value)

    def declare_var(self, key, type):
        assert len(self.scopes) >= 1
        log.debug('declaring var %s' % key)
        self.scopes[-1].declare_var(key, type)

    def lookup_var(self, key):
        for scope in reversed(self.scopes):
            try:
                return scope.lookup_var(key)
            except KeyError:
                pass
        raise KeyError(key)

    def declare_type(self, key, type):
        assert len(self.scopes) >= 1
        log.debug('declaring type %s' % key)
        self.scopes[-1].declare_type(key, type)

    def lookup_type(self, key):
        assert len(self.scopes) >= 1
        for scope in reversed(self.scopes):
            try:
                return scope.lookup_type(key)
            except KeyError:
                pass
        raise KeyError(key)
