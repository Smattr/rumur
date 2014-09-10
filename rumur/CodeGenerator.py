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

    else:
        raise NotImplementedError

def concat(xs):
    return '\n\n'.join(map(generate, xs))

def mangle(symbol):
    return 'user_%s' % symbol

def hang(block):
    lines = block.split('\n')
    indented = lines[0]
    if len(lines) > 1:
        indented += '\n%s' % '\n'.join(map(lambda x: '  %s' % x, lines[1:]))
    return indented
