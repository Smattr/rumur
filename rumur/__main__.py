import argparse, sys
from .Parser import Parser

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

    # try:
    #     env = Environment.Environment()
    #     code = CodeGenerator.generate(env, ast)
    # except Exception as e:
    #     print >>sys.stderr, str(e)
    #     return -1

    # print >>opts.output, code

    return 0

if __name__ == '__main__':
    sys.exit(main())
