#!/usr/bin/env python

import argparse
import re
import sys

def main(args):

  # parse command line arguments
  parser = argparse.ArgumentParser(
    description='convert file contents to a C++ string')
  parser.add_argument('input', type=argparse.FileType('rt'), help='input file')
  parser.add_argument('output', type=argparse.FileType('wt'),
    help='output file')
  options = parser.parse_args(args[1:])

  options.output.write(
    '#include <string>\n'
    '\n'
    'extern const std::string {} =\n'
    '  "'.format(re.sub(r'[^\w\d]', '_', options.input.name)))

  index = 0
  for line in options.input:
    for c in line:

      if c == '"':
        options.output.write('\\"')
        index += 1

      elif c == '\n':
        options.output.write('\\n')
        index += 1

      elif c == '\\':
        options.output.write('\\\\')
        index += 1

      else:
        options.output.write(c)

      index += 1

      if index >= 77:
        options.output.write('"\n  "')
        index = 0

  options.output.write('";')

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv))
