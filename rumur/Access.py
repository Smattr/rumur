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
        return 'read_direct(%s)' % self.name

    def write(self, src, state_symbol):
        return 'write_direct(%(dest)s, %(src)s)' % {
            'dest':self.name,
            'src':src,
        }

class StateSymbol(Symbol):
    def __init__(self, name, type, offset):
        super(StateSymbol, self).__init__(name, type)
        self.offset = offset

    def read(self, state_symbol):
        return 'read_from_state(%(state)s, %(offset)d, %(cardinality)d)' % {
            'state':state_symbol,
            'offset':self.offset,
            'cardinality':self.type.cardinality(),
        }

    def write(self, src, state_symbol):
        return 'write_to_state(%(state)s, %(offset)d, %(cardinality)d, %(src)s)' % {
            'state':state_symbol,
            'offset':self.offset,
            'cardinality':self.type.cardinality(),
            'src':src,
        }
