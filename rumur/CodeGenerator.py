from functools import partial
from Type import Array, Enum, Range, Record, Type

def generate(env, n):
    if n.head == 'constdecl':
        symbol = generate(env, n.tail[0])
        value = hang(generate(env, n.tail[1]))
        return 'static mpz_t %(symbol)s;\n' \
               'static void %(symbol)s_init(void) __attribute__((constructor)) {\n' \
               '  write_direct(%(symbol)s, %(value)s);\n' \
               '}' % locals()

    elif n.head == 'decl':
        return concat(env, n.tail)

    elif n.head == 'expr':
        if n.tail[0].head == 'lbrace':
            assert n.tail[2].head == 'rbrace'
            inner = generate(env, n.tail[1])
            return '(%(inner)s)' % locals()
        elif n.tail[0].head == 'integer_constant':
            return generate(env, n.tail[0])
        elif n.tail[0].head == 'symbol':
            assert n.tail[1].head == 'lbrace'
            assert n.tail[3].head == 'rbrace'
            callee = generate(env, n.tail[0])
            parameters = generate(env, n.tail[2])
            return '%(callee)s(%(parameters)s)' % locals()
        elif n.tail[0].head == 'forall':
            return hol_operation(env, 'forall', n.tail[1], n.tail[2])
        elif n.tail[0].head == 'exists':
            return hol_operation(env, 'exists', n.tail[1], n.tail[2])
        elif n.tail[1].head == 'add':
            return binary_operation(env, 'mpz_add', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'sub':
            return binary_operation(env, 'mpz_sub', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'mul':
            return binary_operation(env, 'mpz_mul', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'div':
            return binary_operation(env, 'mpz_div', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'mod':
            return binary_operation(env, 'mpz_mod', n.tail[0], n.tail[2])
        elif n.tail[0].head == 'not':
            operand = generate(env, n.tail[1])
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
            return binary_operation(env, 'mpz_lor', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'and':
            return binary_operation(env, 'mpz_land', n.tail[0], n.tail[2])
        elif n.tail[1].head =='implies':
            return binary_operation(env, 'mpz_implies', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'lt':
            return binary_operation(env, 'mpz_lt', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'lte':
            return binary_operation(env, 'mpz_lte', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'gt':
            return binary_operation(env, 'mpz_gt', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'gte':
            return binary_operation(env, 'mpz_gte', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'eq':
            return binary_operation(env, 'mpz_eq', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'neq':
            return binary_operation(env, 'mpz_neq', n.tail[0], n.tail[2])
        elif n.tail[1].head == 'question':
            operand1 = generate(env, n.tail[0])
            operand2 = generate(env, n.tail[2])
            operand3 = generate(env, n.tail[4])
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

    elif n.head == 'function':
        # The [1:-1] slices below are because we know the first and last token
        # of a function production.
        symbol = n.tail[0]
        formals = filter(lambda x: x.head == 'formal', n.tail[1:-1])
        typeexpr = filter(lambda x: x.head == 'typeexpr', n.tail[1:-1])
        assert len(typeexpr) == 1
        return_type = typeexpr[0]
        decls = filter(lambda x: x.head == 'decl', n.tail[1:-1])
        body = filter(lambda x: x.head == 'stmts', n.tail[1:-1])
        stmts = body[0] if len(body) > 0 else None
        return make_function(env, symbol, formals, return_type, decls, stmts)

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
        return generate(env, n.tail[0])

    elif n.head == 'procdecl':
        return generate(env, n.tail[0])

    elif n.head == 'procedure':
        # See comments for 'function'
        symbol = n.tail[0]
        formals = filter(lambda x: x.head == 'formal', n.tail[1:-1])
        return_type = None
        decls = filter(lambda x: x.head == 'decl', n.tail[1:-1])
        body = filter(lambda x: x.head == 'stmts', n.tail[1:-1])
        stmts = body[0] if len(body) > 0 else None
        return make_function(env, symbol, formals, return_type, decls, stmts)

    elif n.head == 'program':
        env.new_scope()
        return concat(env, n.tail)

    elif n.head == 'putstmt':
        if n.tail[0].head == 'expr':
            expr = generate(env, n.tail[0])
            return 'do {\n' \
                   '  mpz_t x = %(expr)s;\n' \
                   '  gmp_printf("%%Zu", x);\n' \
                   '  mpz_clear(x);\n' \
                   '} while (0);' % locals()
        else:
            assert n.tail[0].head == 'string'
            s = n.tail[0].tail[0][1:-1] # strip quotes
            # XXX: Escape this safely.
            return 'printf("%%s", "%(s)s");' % locals()

    elif n.head == 'returnstmt':
        code = ''
        if len(n.tail) > 0:
            expr = generate(env, n.tail[0])
            # Note that we assume the variable 'ret' is available.
            code += 'do {\n' \
                    '  mpz_t x = %(expr)s;\n' \
                    '  mpz_set(ret, x);\n' \
                    '  mpz_clear(x);\n' \
                    '} while (0);\n' % locals()
        # See make_function() for where this label is emitted.
        code += 'goto coda;'
        return code

    elif n.head == 'symbol':
        return mangle(n.tail[0])

    elif n.head == 'typedecl':
        symbol = generate(env, n.tail[0])
        type = decode_type(env, n.tail[1])
        env.typedef(symbol, type)
        return '' # No code required

    elif n.head == 'vardecl':
        code = ''
        type = decode_type(env, n.tail[-1])
        for s in n.tail[:-1]:
            symbol = generate(env, s)
            env.declare(symbol, type, None)
            if not env.bare():
                # We only need to generate code if this is not a state variable.
                code += 'mpz_t %(symbol)s;\n' \
                        'mpz_init(%(symbol)s);' % locals()
        return code

    elif n.head == 'whilestmt':
        expr = generate(env, n.tail[0])
        if n.tail[1].head == 'stmts':
            stmts = cat(env, n.tail[1].tail)
        else:
            stmts = ''
        return 'while (({\n' \
               '    mpz_t x = %(expr)s;\n' \
               '    int r;\n' \
               '    r = (mpz_cmp_ui(x, 0) != 0);\n' \
               '    mpz_clear(x);\n' \
               '    r;\n' \
               '  })) {\n' \
               '  %(stmts)s\n' \
               '}' % locals()

    else:
        raise NotImplementedError

def binary_operation(env, op, x, y):
    operand1 = generate(env, x)
    operand2 = generate(env, y)
    return '({\n' \
           '  mpz_t x = %(operand1)s;\n' \
           '  mpz_t y = %(operand2)s;\n' \
           '  %(op)s(x, y);\n' \
           '  mpz_clear(y);\n' \
           '  x;\n' \
           '})' % locals()

def hol_operation(env, binder, quantifier, body):

    if binder == 'forall':
        initial_value = 1
        modifier = '!'
        abort_value = 0
    else:
        assert binder == 'exists'
        initial_value = 0
        modifier = ''
        abort_value = 1

    iterator = generate(env, quantifier.tail[0])
    if quantifier.tail[1].head == 'typeexpr':
        # First case of quantifier
        lower_init = 'mpz_init_set_ui(lower, 0);' #TODO
        upper_init = 'mpz_init_set_ui(upper, 0);' #TODO
        increment_init = 'mpz_init_set_ui(increment, 1);'
    else:
        l = generate(env, quantifier.tail[1])
        lower_init = 'mpz_init_set(lower, %(l)s);' % locals()
        u = generate(env, quantifier.tail[2])
        upper_init = 'mpz_init_set(upper, %(u)s);' % locals()
        if len(quantifier.tail) == 4:
            # We have an explicit step clause
            i = generate(env, quantifier.tail[3])
            increment_init = 'mpz_init_set(increment, %(i)s);' % locals()
        else:
            increment_init = 'mpz_init_set_ui(increment, 1);'

    # Adjust indentation in case any of these are more than one line.
    lower_init = hang(lower_init)
    upper_init = hang(upper_init)
    increment_init = hang(increment_init)

    expr = generate(env, body)

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

def decode_type(env, typeexpr):
    if typeexpr.tail[0].head == 'symbol':
        symbol = generate(env, typeexpr.tail[0])
        referent = env.lookup(symbol)
        if referent is None:
            raise Exception('type alias refers to non-existent type %s' % typeexpr.tail[0].head)
        elif not isinstance(referent, Type):
            raise Exception('type alias refers to %s which is not a type' % typeexpr.tail[0].head)
        return referent

    elif typeexpr.tail[0].head == 'expr':
        # FIXME: Range actually expects constants as its limits
        min = generate(env, typeexpr.tail[0])
        max = generate(env, typeexpr.tail[1])
        return Range(min, max)

    elif typeexpr.tail[0].head == 'enum':
        members = map(partial(generate, env), typeexpr.tail[1:])
        return Enum(members)

    elif typeexpr.tail[0].head == 'record':
        members = {}
        for vardecl in typeexpr.tail[1:-1]:
            type = decode_type(env, vardecl.tail[-1])
            for symbol in vardecl.tail[:-1]:
                s = generate(env, symbol)
                if s in members:
                    raise Exception('duplicate record members %s given' % symbol.tail[0].head)
                members[s] = type
        return Record(members)

    else:
        assert typeexpr.tail[0].head == 'array'
        index_type = decode_type(env, typeexpr.tail[1])
        member_type = decode_type(env, typeexpr.tail[2])
        return Array(index_type, member_type)

def make_function(env, symbol, formals, return_type, decls, stmts):
    function = generate(env, symbol)
    env.new_scope()
    params = []
    if return_type is not None:
        params.append('mpz_t ret')
    prelude = []
    coda = []
    for formal in formals:
        if formal.tail[0].head == 'var':
            writable = True
            symbols = formal.tail[1:-1]
        else:
            writable = False
            symbols = formal.tail[:-1]
        type = decode_type(env, formal.tail[-1])
        for s in symbols:
            mangled = generate(env, s)
            env.declare(mangled, type, None)
            p = 'p_%s' % mangled
            params.append('mpz_t %s' % p)
            if writable:
                prelude.append('mpz_t %(mangled)s = %(p)s;' % locals())
            else:
                prelude.append('mpz_t %s;' % mangled)
                prelude.append('mpz_set(%(mangled)s, %(p)s);' % locals())
                coda.append('mpz_clear(%s);' % mangled)

    for decl in decls:
        code = generate(env, decl)
        # XXX: What happens if the decl calls mpz_init and we need to clear this later?
        if code:
            prelude.append(code)

    if stmts is None:
        body = ''
    else:
        body = generate(env, stmts)

    prelude = '\n  '.join(prelude)
    params = ', '.join(params)
    coda = '\n  '.join(coda)
    env.pop_scope()
    return 'void %(function)s(%(params)s) {\n' \
           '  %(prelude)s\n' \
           '  %(body)s\n' \
           'coda:\n' \
           '  %(coda)s\n' \
           '}' % locals()

def concat(env, xs):
    s = ''
    for x in xs:
        s += '%s\n\n' % generate(env, x)
    if s != '':
        s = s[:-2]
    return s

def cat(env, xs):
    s = ''
    for x in xs:
        s += '%s\n' % generate(env, x)
    if s != '':
        s = s[:-2]
    return s

def mangle(symbol):
    return 'user_%s' % symbol

def hang(block):
    lines = block.split('\n')
    indented = lines[0]
    if len(lines) > 1:
        indented += '\n%s' % '\n'.join(map(lambda x: '  %s' % x, lines[1:]))
    return indented
