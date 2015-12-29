import argparse, sys
from CodeGenerator import to_code
from ConstantFolding import constant_fold
from IRGenerator import to_ir
from Parser import Parser

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
    ir = to_ir(ast)

    ast = run_to_fixed_point(ast, constant_fold)

    # program = lift(ast)

    # try:
    #     env = Environment.Environment()
    #     code = CodeGenerator.generate(env, ast)
    # except Exception as e:
    #     print >>sys.stderr, str(e)
    #     return -1

    # print >>opts.output, code

    opts.output.write(''.join(to_code(ast)))

    return 0

if __name__ == '__main__':
    sys.exit(main())
