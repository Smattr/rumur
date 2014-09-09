import math

class Type(object):
    def cardinality(self):
        raise NotImplementedError
    def offset(self, path):
        assert isinstance(path, list)
        raise NotImplementedError
    def terminal(self, path):
        assert isinstance(path, list)
        raise NotImplementedError

class PrimitiveType(Type):
    def offset(self, path):
        assert isinstance(path, list)
        if len(path) != 0:
            raise ValueError
        return 0
    def terminal(self, path):
        assert isinstance(path, list)
        if len(path) != 0:
            raise ValueError
        return self

class Boolean(PrimitiveType):
    def cardinality(self):
        # True, False and Undefined
        return 3
boolean = Boolean()

class Range(PrimitiveType):
    def __init__(self, minimum, maximum):
        assert isinstance(minimum, int)
        self.minimum = minimum
        assert isinstance(maximum, int)
        assert maximum >= minimum
        self.maximum = maximum
    def cardinality(self):
        return self.maximum - self.minimum + 1

class Enum(PrimitiveType):
    def __init__(self, values):
        assert isinstance(values, list)
        for v in values:
            assert isinstance(v, str)
        self.values = values
    def cardinality(self):
        return len(self.values)

class Array(Type):
    def __init__(self, index_type, value_type):
        assert isinstance(index_type, Type)
        assert isinstance(value_type, Type)
        if not isinstance(index_type, PrimitiveType):
            raise TypeError
        self.index_type = index_type
        self.value_type = value_type
    def cardinality(self):
        return self.index_type.cardinality() * self.value_type.cardinality()
    def offset(self, path):
        assert isinstance(path, list)
        if len(path) == 0:
            return 0
        assert isinstance(path[0], PathAtom)
        if not isinstance(path[0], ArrayIndex):
            raise TypeError
        #XXX

    def terminal(self, path):
        assert isinstance(path, list)
        if len(path) == 0:
            return self

class Alias(Type):
    def __init__(self, referent):
        assert isinstance(referent, Type):
        self.referent = referent
    def cardinality(self):
        return self.referent.cardinality()

class Record(Type):
    def __init__(self, members):
        assert isinstance(members, dict)
        for m in members.values():
            assert isinstance(m, Type)
        self.members = members
    def cardinality(self):
        return sum(map(Type.cardinality, self.members.values()))

class PathAtom(object):
    def __init__(self, value):
        self.value = value

class ArrayIndex(PathAtom):
    pass

class RecordMember(PathAtom):
    pass
