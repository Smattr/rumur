'''
Encapsulation of multi-precision integers.
'''

# Arbitrarily assume 32-bits for now.
UINT_MAX = 2 ** 32 - 1

def op(operation, operand1, operand2):
    return '({\n' \
           '  mpz_t x = %(operand1)s;\n' \
           '  mpz_t y = %(operand2)s;\n' \
           '  mpz_%(operation)s(x, x, y);\n' \
           '  mpz_clear(y);\n' \
           '  x;\n' \
           '})' % {
               'operand1':operand1.to_mpz(),
               'operand2':operand2.to_mpz(),
               'operation':operation,
           }

def cmpop(operator, operand1, operand2):
    return '({\n' \
           '  mpz_t x = %(operand1)s;\n' \
           '  mpz_t y = %(operand2)s;\n' \
           '  mpz_set(x, mpz_cmp(x, y) %(operator)s 0);\n' \
           '  mpz_clear(y);\n' \
           '  x;\n' \
           '})' % {
               'operand1':operand1.to_mpz(),
               'operand2':operand2.to_mpz(),
               'operator':operator,
           }

class MPTerm(object):
    def __init__(self, value):
        self.value = value

    def to_ui(self):
        return '({\n' \
               '  mpz_t x = %s;\n' \
               '  if (mpz_cmp_ui(x, UINT_MAX) > 0) {\n' \
               '    /* x will not fit in a uint */\n' \
               '    exception(...);\n' \
               '  }\n' \
               '  unsigned int y = mpz_get_ui(x);\n' \
               '  mpz_clear(x);\n' \
               '  y;\n' \
               '})' % self.value

    def to_mpz(self):
        return self.value

    def __add__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        return MPTerm(op('add', self, other))

    def __sub__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        return MPTerm(op('sub', self, other))

    def __mul__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        return MPTerm(op('mul', self, other))

    def __div__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        return MPTerm(op('div', self, other))

    def __mod__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        return MPTerm(op('mod', self, other))

    def __and__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        return MPTerm(op('land', self, other))

    def __or__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        return MPTerm(op('lor', self, other))

    def implies(self, other):
        if isinstance(other, int):
            other = Literal(other)
        code = '({\n' \
               '  mpz_t x = %(operand1)s;\n' \
               '  mpz_t y = %(operand2)s;\n' \
               '  if (mpz_cmp_ui(x, 0) != 0) {\n' \
               '    mpz_set(x, y);\n' \
               '  }\n' \
               '  mpz_clear(y);\n' \
               '  x;\n' \
               '})' % {
                   'operand1':self.to_mpz(),
                   'operand2':other.to_mpz(),
               }
        return MPTerm(code)

    def invert(self):
        code = '({\n' \
               '  mpz_t x = %(value)s;\n' \
               '  mpz_set(x, mpz_cmp_ui(x, 0) == 0);\n' \
               '  x;\n' \
               '})'
        return MPTerm(code)

    def __eq__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        return MPTerm(cmpop('==', self, other))

    def __ne__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        return MPTerm(cmpop('!=', self, other))

    def __ge__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        return MPTerm(cmpop('>=', self, other))

    def __gt__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        return MPTerm(cmpop('>', self, other))

    def __lt__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        return MPTerm(cmpop('<', self, other))

    def __le__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        return MPTerm(cmpop('<=', self, other))

    def tricond(self, operand2, operand3):
        if isinstance(operand2, int):
            operand2 = Literal(operand2)
        if isinstance(operand3, int):
            operand3 = Literal(operand3)
        code = '({\n' \
               '  mpz_t x = %(operand1)s;\n' \
               '  mpz_t y = %(operand2)s;\n' \
               '  mpz_t z = %(operand3)s;\n' \
               '  if (mpz_cmp_ui(x, 0) == 0) {\n' \
               '    mpz_set(y, z);\n' \
               '  }\n' \
               '  mpz_clear(x);\n' \
               '  mpz_clear(z);\n' \
               '  y;\n' \
               '})' % {
                   'operand1':self.to_mpz(),
                   'operand2':operand2.to_mpz(),
                   'operand3':operand3.to_mpz(),
               }
        return MPTerm(code)

class Literal(MPTerm):
    '''This class is mainly here as an optimisation. It allows us to do
    generation-time constant folding within expressions.'''

    def __init__(self, value):
        assert value <= UINT_MAX, \
            'trying to store a literal that won\'t fit in an unsigned int'
        self.value = value

    def to_ui(self):
        return '%uu' % self.value

    def to_mpz(self):
        return '({\n' \
               '  mpz_t x;\n' \
               '  mpz_init_set_ui(x, %uu);\n' \
               '  x;\n' \
               '})' % self.value

    def __add__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        if isinstance(other, Literal) and self.value + other.value <= UINT_MAX:
            # Optimisation: constant folding
            return Literal(self.value + other.value)
        return super(Literal, self).__add__(other)

    def __sub__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        if isinstance(other, Literal):
            return Literal(self.value - other.value)
        return super(Literal, self).__sub__(other)

    def __mul__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        if isinstance(other, Literal) and self.value * other.value <= UINT_MAX:
            return Literal(self.value * other.value)
        return super(Literal, self).__mul__(other)

    def __div__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        if isinstance(other, Literal):
            return Literal(self.value / other.value)
        return super(Literal, self).__div__(other)

    def __mod__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        if isinstance(other, Literal):
            return Literal(self.value % other.value)
        return super(Literal, self).__mod__(other)

    def __and__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        if isinstance(other, Literal):
            return Literal(self.value and other.value)
        return super(Literal, self).__and__(other)

    def __or__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        if isinstance(other, Literal):
            return Literal(self.value or other.value)
        return super(Literal, self).__or__(other)

    def implies(self, other):
        if isinstance(other, int):
            other = Literal(other)
        if isinstance(other, Literal):
            return Literal((not self.value) or other.value)
        return super(Literal, self).implies(other)

    def invert(self):
        if self.value == 0:
            return Literal(1)
        return Literal(0)

    def __eq__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        if isinstance(other, Literal):
            return Literal(self.value == other.value)
        return super(Literal, self).__eq__(other)

    def __ne__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        if isinstance(other, Literal):
            return Literal(self.value != other.value)
        return super(Literal, self).__ne__(other)

    def __ge__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        if isinstance(other, Literal):
            return Literal(self.value >= other.value)
        return super(Literal, self).__ge__(other)

    def __gt__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        if isinstance(other, Literal):
            return Literal(self.value > other.value)
        return super(Literal, self).__gt__(other)

    def __lt__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        if isinstance(other, Literal):
            return Literal(self.value < other.value)
        return super(Literal, self).__lt__(other)

    def __le__(self, other):
        if isinstance(other, int):
            other = Literal(other)
        if isinstance(other, Literal):
            return Literal(self.value <= other.value)
        return super(Literal, self).__le__(other)

    def tricond(self, operand2, operand3):
        if isinstance(operand2, int):
            operand2 = Literal(operand2)
        if isinstance(operand3, int):
            operand3 = Literal(operand3)
        if self.value != 0:
            return operand2
        return operand3
