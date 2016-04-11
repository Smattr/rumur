import argparse, logging, sys
from CodeGenerator import to_code
from IRGenerator import to_ir
from Log import log
from Parser import Parser
from OptCF import constant_fold
from OptSR import strength_reduce

def run_to_fixed_point(ast, *transformers):
    modified = True
    while modified:
        modified = False
        for t in transformers:
            m, ast = t(ast)
            modified |= m
    return ast

def main():
    parser = argparse.ArgumentParser(
        description='a Murphi model optimiser')
    parser.add_argument('--output', '-o', metavar='FILE',
        type=argparse.FileType('w'), default=sys.stdout,
        help='output file')
    parser.add_argument('input', type=argparse.FileType('r'),
        help='input specification')
    opts = parser.parse_args()

    # XXX
    log.setLevel(logging.DEBUG)

    # Construct the parser.
    try:
        p = Parser()
    except Exception as e:
        print >>sys.stderr, str(e)
        return -1

    # Parse the input.
    try:
        ast = p.parse(opts.input.read())
    except Exception as e:
        print >>sys.stderr, str(e)
        return -1

    # Transform the AST into an intermediate representation.
    global_scope, ir = to_ir(ast)

    # Run optimisations.
    OPT_PIPELINE = [
        strength_reduce,
        constant_fold,
    ]

    for p in OPT_PIPELINE:
        ir = p(ir)

    #ast = run_to_fixed_point(ast, constant_fold)

    # program = lift(ast)

    # try:
    #     env = Environment.Environment()
    #     code = CodeGenerator.generate(env, ast)
    # except Exception as e:
    #     print >>sys.stderr, str(e)
    #     return -1

    # print >>opts.output, code

    #opts.output.write(''.join(to_code(env, ast)))

    code = to_code(global_scope, ir)

    for fragment in code:
        opts.output.write(fragment)

    return 0

if __name__ == '__main__':
    sys.exit(main())
