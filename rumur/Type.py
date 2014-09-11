class Type(object):
    def cardinality(self):
        raise NotImplementedError
    def subsumes(self, other):
        raise NotImplementedError
    def __ge__(self, other):
        return self.subsumes(other)
    def equals(self, other):
        raise NotImplementedError
    def __eq__(self, other):
        return self.equals(other)

class Enum(Type):
    def __init__(self, members):
        self.members = members
    def cardinality(self):
        return '({\n' \
               '  mpz_t x;\n' \
               '  mpz_init_set_ui(x, %d);\n' \
               '  x;\n' \
               '})' % (len(self.members) + 1)
    def subsumes(self, other):
        if isinstance(other, Unconstrained):
            return None
        # In the case of a range, we can't just test the cardinality because
        # the range may not start from 0.
        return (isinstance(other, Enum) and \
                len(other.members) <= len(self.members)) or \
               (isinstance(other, Range) and \
                other.min >= 0 and other.max < len(self.members))
    def equals(self, other):
        return isinstance(other, Enum) and \
            len(self.members) == len(other.members)

boolean = Enum(['false', 'true'])

class Range(Type):
    def __init__(self, min, max):
        self.min = min
        self.max = max
    def cardinality(self):
        return '({\n' \
               '  mpz_t x = %(min)s;\n' \
               '  mpz_t y = %(max)s;\n' \
               '  mpz_sub(y, y, x);\n' \
               '  mpz_clear(x);\n' \
               '  mpz_add_ui(y, y, 2);\n' \
               '  y;\n' \
               '})' % {
                   'min':self.min,
                   'max':self.max,
               }
    def subsumes(self, other):
        if isinstance(other, Unconstrained):
            return None
        return (isinstance(other, Range) and \
                other.min >= self.min and other.max <= self.max) or \
               (isinstance(other, Enum) and \
                self.min <= 0 and self.max >= len(other.members) - 1)
    def equals(self, other):
        return isinstance(other, Range) and \
            self.min == other.min and self.max == other.max

class Array(Type):
    def __init__(self, index_type, member_type):
        self.index_type = index_type
        self.member_type = member_type
    def cardinality(self):
        return '({\n' \
               '  mpz_t x = %(index_card)s;\n' \
               '  mpz_t y = %(member_card)s;\n' \
               '  mpz_sub_ui(x, x, 1);\n' \
               '  mpz_mul(x, x, y);\n' \
               '  mpz_clear(y);\n' \
               '  x;\n' \
               '})' % {
                   'index_card':self.index_type.cardinality(),
                   'member_card':self.member_type.cardinality(),
               }
    def subsumes(self, other):
        return False
    def equals(self, other):
        return isinstance(other, Array) and \
            self.index_type == other.index_type and \
            self.member_type == other.member_type

class Record(Type):
    def __init__(self, members):
        self.members = members
    def cardinality(self):
        card = '({\n' \
               '  mpz_t x;\n' \
               '  mpz_init_set_ui(x, 1);\n' \
               '  x;\n' \
               '})'
        for v in self.members.values():
            member_card = v[1].cardinality()
            card = '({\n' \
                   '  mpz_t x = %(card)s;\n' \
                   '  mpz_t y = %(member_card)s;\n' \
                   '  mpz_mul(x, x, y);\n' \
                   '  mpz_clear(y);\n' \
                   '  x;\n' \
                   '})' % locals()
    def subsumes(self, other):
        return False
    def equals(self, other):
        return isinstance(other, Record) and \
            self.members == other.members

class Unconstrained(Type):
    def cardinality(self):
        raise TypeError
    def subsumes(self, other):
        return False
    def equals(self, other):
        return False
unconstrained = Unconstrained()
