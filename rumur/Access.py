class Symbol(object):
    def __init__(self, name, type):
        self.name = name
        self.type = type

    def read(self, state_symbol):
        raise NotImplementedError

    def write(self, src, state_symbol):
        raise NotImplementedError

class DirectSymbol(Symbol):
    def __init__(self, name, type):
        super(DirectSymbol, self).__init__(name, type)

    def read(self, state_symbol):
        return 'r(%(root)s, 1, %(cardinality)d)' % {
            'root':self.name,
            'cardinality':self.type.cardinality(),
        }

    def write(self, src, state_symbol):
        return 'w(%(dest)s, 1, %(cardinality)s, %(src)s)' % {
            'dest':self.name,
            'cardinality':self.type.cardinality(),
            'src':src,
        }

class StateSymbol(Symbol):
    def __init__(self, name, type, offset):
        super(StateSymbol, self).__init__(name, type)
        self.offset = offset

    def read(self, state_symbol):
        return 'r(%(state)s, %(offset)d, %(cardinality)d)' % {
            'state':state_symbol,
            'offset':self.offset,
            'cardinality':self.type.cardinality(),
        }

    def write(self, src, state_symbol):
        return 'w(%(state)s, %(offset)d, %(cardinality)d, %(src)s)' % {
            'state':state_symbol,
            'offset':self.offset,
            'cardinality':self.type.cardinality(),
            'src':src,
        }
