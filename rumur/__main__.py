import argparse, sys
import CodeGenerator, Environment, Parser

def main():
    parser = argparse.ArgumentParser(
        description='a model checker generator')
    parser.add_argument('--output', '-o', metavar='FILE',
        type=argparse.FileType('w'), default=sys.stdout,
        help='output file')
    parser.add_argument('input', type=argparse.FileType('r'),
        help='input specification')
    opts = parser.parse_args()

    try:
        p = Parser.Parser()
    except Exception as e:
        print >>sys.stderr, str(e)
        return -1

    try:
        ast = p.parse(opts.input.read())
    except Exception as e:
        print >>sys.stderr, str(e)
        return -1

    try:
        env = Environment.Environment()
        code = CodeGenerator.generate(env, ast)
    except Exception as e:
        print >>sys.stderr, str(e)
        return -1

    print >>opts.output, code

    return 0

if __name__ == '__main__':
    sys.exit(main())
