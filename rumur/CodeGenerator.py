def generate(n):
    if n.head == 'constdecl':
        symbol = generate(n.tail[0])
        value = hang(generate(n.tail[1]))
        return 'static mpz_t %(symbol)s;\n' \
               'static void %(symbol)s_init(void) __attribute__((constructor)) {\n' \
               '  mpz_t x = %(value)s;\n' \
               '  mpz_init_set(%(symbol)s, x);\n' \
               '  mpz_clear(x);\n' \
               '}' % locals()

    elif n.head == 'decl':
        return concat(n.tail)

    elif n.head == 'expr':
        if len(n.tail) == 1:
            return generate(n.tail[0])
        elif n.tail[0].head == 'lbrace':
            assert n.tail[2].head == 'rbrace'
            inner = generate(n.tail[1])
            return '(%(inner)s)' % locals()
        elif n.tail[0].head == 'symbol':
            assert n.tail[1].head == 'lbrace'
            assert n.tail[3].head == 'rbrace'
            callee = generate(n.tail[0])
            parameters = generate(n.tail[2])
            return '%(callee)s(%(parameters)s)' % locals()
        elif n.tail[0].head == 'forall':
            return hol_operation('forall', n.tail[1], n.tail[2])
        elif n.tail[0].head == 'exists':
            return hol_operation('exists', n.tail[1], n.tail[2])
        elif n.tail[1].head == 'add':
            return binary_operation('mpz_add', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'sub':
            return binary_operation('mpz_sub', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'mul':
            return binary_operation('mpz_mul', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'div':
            return binary_operation('mpz_div', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'mod':
            return binary_operation('mpz_mod', n.tail[0], n.tail[2])
        elif n.tail[0].head == 'not':
            operand = generate(n.tail[1])
            return '({\n' \
                   '  mpz_t x = %(operand)s;\n' \
                   '  if (mpz_cmp_ui(x, 0) == 0) {\n' \
                   '    mpz_set_ui(x, 1);\n' \
                   '  } else {\n' \
                   '    mpz_set_ui(x, 0);\n' \
                   '  }\n' \
                   '  x;\n' \
                   '})' % locals()
        elif n.tail[1].head == 'or':
            return binary_operation('mpz_lor', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'and':
            return binary_operation('mpz_land', n.tail[0], n.tail[2])
        elif n.tail[1].head =='implies':
            return binary_operation('mpz_implies', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'lt':
            return binary_operation('mpz_lt', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'lte':
            return binary_operation('mpz_lte', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'gt':
            return binary_operation('mpz_gt', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'gte':
            return binary_operation('mpz_gte', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'eq':
            return binary_operation('mpz_eq', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'neq':
            return binary_operation('mpz_neq', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'question':
            operand1 = generate(n.tail[0])
            operand2 = generate(n.tail[2])
            operand3 = generate(n.tail[4])
            return '({\n' \
                   '  mpz_t x = %(operand1)s;\n' \
                   '  mpz_t y = %(operand2)s;\n' \
                   '  mpz_t z = %(operand3)s;\n' \
                   '  if (mpz_cmp_ui(x, 0) == 0) {\n' \
                   '    mpz_set(y, z);\n' \
                   '  }\n' \
                   '  mpz_clear(x);\n' \
                   '  mpz_clear(z);\n' \
                   '  y;\n' \
                   '})' % locals()
        raise NotImplementedError


    elif n.head == 'integer_constant':
        value = n.tail[0]
        return '({\n' \
               '  mpz_t x;\n' \
               '  mpz_init_set_ui(x, %(value)s);\n' \
               '  x;\n' \
               '})' % locals()

    elif n.head == 'start':
        if len(n.tail) != 1:
            raise Exception('multiple initial tokens')
        return generate(n.tail[0])

    elif n.head == 'program':
        return concat(n.tail)

    elif n.head == 'symbol':
        return mangle(n.tail[0])

    elif n.head == 'whilestmt':
        expr = generate(n.tail[0])
        if n.tail[1].head == 'stmts':
            stmts = cat(n.tail[1].tail)
        else:
            stmts = ''
        return 'while (({\n' \
               '    mpz_t x = %(expr)s;\n' \
               '    int r;\n' \
               '    if (mpz_cmp_ui(x, 0) != 0) {\n' \
               '      r = 1;\n' \
               '    } else {\n' \
               '      r = 0;\n' \
               '    }\n' \
               '    mpz_clear(x);\n' \
               '    r;\n' \
               '  })) {\n'
               '  %(stmts)s\n' \
               '}' % locals()

    else:
        raise NotImplementedError

def binary_operation(op, x, y):
    operand1 = generate(x)
    operand2 = generate(y)
    return '({\n' \
           '  mpz_t x = %(operand1)s;\n' \
           '  mpz_t y = %(operand2)s;\n' \
           '  %(op)s(x, y);\n' \
           '  mpz_clear(y);\n' \
           '  x;\n' \
           '})' % locals()

def hol_operation(binder, quantifier, body):

    if binder == 'forall':
        initial_value = 1
        modifier = '!'
        abort_value = 0
    else:
        assert binder == 'exists'
        initial_value = 0
        modifier = ''
        abort_value = 1

    iterator = generate(quantifier.tail[0])
    if quantifier.tail[1].head == 'typeexpr':
        # First case of quantifier
        lower_init = 'mpz_init_set_ui(lower, 0);' #TODO
        upper_init = 'mpz_init_set_ui(upper, 0);' #TODO
        increment_init = 'mpz_init_set_ui(increment, 1);'
    else:
        l = generate(quantifier.tail[1])
        lower_init = 'mpz_init_set(lower, %(l)s);' % locals()
        u = generate(quantifier.tail[2])
        upper_init = 'mpz_init_set(upper, %(u)s);' % locals()
        if len(quantifier.tail) == 4:
            # We have an explicit step clause
            i = generate(quantifier.tail[3])
            increment_init = 'mpz_init_set(increment, %(i)s);' % locals()
        else:
            increment_init = 'mpz_init_set_ui(increment, 1);'

    # Adjust indentation in case any of these are more than one line.
    lower_init = hang(lower_init)
    upper_init = hang(upper_init)
    increment_init = hang(increment_init)

    expr = generate(body)

    return '({\n' \
           '  mpz_t %(iterator)s;\n' \
           '  mpz_t r;\n' \
           '  mpz_set_ui(r, %(initial_value)d);\n' \
           '  mpz_t lower, upper, increment;\n' \
           '  %(increment_init)s\n' \
           '  %(lower_init)s\n' \
           '  %(upper_init)s\n' \
           '  for (mpz_init_set(%(iterator)s, lower);\n' \
           '       mpz_cmp(%(iterator)s, upper) < 0;\n' \
           '       mpz_add(%(iterator)s, increment)) {\n' \
           '    mpz_t x = %(expr)s;\n' \
           '    if (%(modifier)s(mpz_cmp_ui(x, 0) != 0)) {\n' \
           '      mpz_clear(x);\n' \
           '      mpz_set_ui(r, %(abort_value)d);\n' \
           '      break;\n' \
           '    }\n' \
           '    mpz_clear(x);\n' \
           '  }\n' \
           '  mpz_clear(%(iterator)s);\n' \
           '  mpz_clear(%(lower)s);\n' \
           '  mpz_clear(%(upper)s);\n' \
           '  mpz_clear(%(increment)s);\n' \
           '  r;\n' \
           '})' % locals()

def concat(xs):
    return '\n\n'.join(map(generate, xs))

def cat(xs):
    return '\n'.join(map(generate, xs))

def mangle(symbol):
    return 'user_%s' % symbol

def hang(block):
    lines = block.split('\n')
    indented = lines[0]
    if len(lines) > 1:
        indented += '\n%s' % '\n'.join(map(lambda x: '  %s' % x, lines[1:]))
    return indented
