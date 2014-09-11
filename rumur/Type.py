class Type(object):
    def cardinality(self):
        raise NotImplementedError

class Enum(Type):
    def __init__(self, members):
        self.members = members
    def cardinality(self):
        return '({\n' \
               '  mpz_t x;\n' \
               '  mpz_init_set_ui(x, %d);\n' \
               '  x;\n' \
               '})' % (len(self.members) + 1)

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

class Unconstrained(Type):
    def cardinality(self):
        raise TypeError
unconstrained = Unconstrained()
