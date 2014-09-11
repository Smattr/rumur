from MPZ import Literal

class Type(object):
    def cardinality(self):
        raise NotImplementedError

class Enum(Type):
    def __init__(self, members):
        self.members = members
    def cardinality(self):
        return Literal(len(self.members) + 1)

boolean = Enum(['false', 'true'])

class Range(Type):
    def __init__(self, min, max):
        self.min = min
        self.max = max
    def cardinality(self):
        return self.max - self.min + 2

class Array(Type):
    def __init__(self, index_type, member_type):
        self.index_type = index_type
        self.member_type = member_type
    def cardinality(self):
        index_card = self.index_type.cardinality();
        member_card = self.member_type.cardinality();
        return (index_card - 1) * member_card

class Record(Type):
    def __init__(self, members):
        self.members = members
    def cardinality(self):
        card = Literal(1)
        for v in self.members.values():
            member_card = v[1].cardinality()
            card *= member_card

class Unconstrained(Type):
    def cardinality(self):
        raise TypeError
unconstrained = Unconstrained()
