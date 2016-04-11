import abc, collections, six
from IR import TypeConstant, Lit, TypeEnum
from Log import log
from RumurError import RumurError

class Variable(object):
    def __init__(self, typ, writable=True, offsetof=None):
        self.typ = typ
        self.writable = writable
        self.offsetof = offsetof

class Scope(six.with_metaclass(abc.ABCMeta, object)):

    @abc.abstractmethod
    def declare_constant(self, key, value):
        raise NotImplementedError

    @abc.abstractmethod
    def declare_var(self, key, typ, writable=True):
        raise NotImplementedError

    @abc.abstractmethod
    def declare_type(self, key, typ):
        raise NotImplementedError

    @abc.abstractmethod
    def lookup_var(self, key):
        raise NotImplementedError

    @abc.abstractmethod
    def lookup_type(self, key):
        raise NotImplementedError

class BasicScope(Scope):

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
        self.constants[key] = Variable(TypeConstant(value), False)

    def declare_var(self, key, typ, writable):
        if key in self.constants:
            raise RumurError('%s conflicts with a constant declared in the '
                'same scope' % key)
        if key in self.vars:
            raise RumurError('%s conflicts with a variable declared in the '
                'same scope' % key)
        self.vars[key] = Variable(typ, writable)

    def lookup_var(self, key):
        try:
            return self.constants[key]
        except KeyError:
            return self.vars[key]

    def declare_type(self, key, typ):
        if key in self.types:
            raise RumurError('%s conflicts with a type declared in the same '
                'scope' % key)
        self.types[key] = typ

    def lookup_type(self, key):
        return self.types[key]

class OrderedScope(BasicScope):

    def __init__(self):
        self.constants = collections.OrderedDict()
        self.types = collections.OrderedDict()
        self.vars = collections.OrderedDict()

class StateScope(BasicScope):

    def __init__(self):
        super(StateScope, self).__init__()
        self._offset = 0

    def declare_var(self, key, typ, writable):
        if key in self.constants:
            raise RumurError('%s conflicts with a constant declared in the '
                'same scope' % key)
        if key in self.vars:
            raise RumurError('%s conflicts with a variable declared in the '
                'same scope' % key)
        self.vars[key] = Variable(typ, writable, self._offset)
        self._offset += typ.cardinality()

class Environment(object):

    def __init__(self):
        self.scopes = []
        self.scopes.append(StateScope())
        self.declare_type('boolean', TypeEnum(('false', 'true'), None))
        self.declare_constant('true', True)
        self.declare_constant('false', False)

    def open_scope(self):
        self.scopes.append(Scope())

    def open_ordered_scope(self):
        self.scopes.append(OrderedScope())

    def close_scope(self):
        self.scopes.pop()

    def declare_constant(self, key, value):
        assert len(self.scopes) >= 1
        log.debug('declaring constant %s' % key)
        self.scopes[-1].declare_constant(key, value)

    def declare_var(self, key, type, writable):
        assert len(self.scopes) >= 1
        log.debug('declaring var %s' % key)
        self.scopes[-1].declare_var(key, type, writable)

    def lookup_var(self, key):
        for index, scope in enumerate(reversed(self.scopes)):
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

    @property
    def scope(self):
        assert len(self.scopes) >= 1
        return self.scopes[-1]
