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

  array = re.sub(r'[^\w\d]', '_', options.input.name)
  size = '{}_len'.format(array)

  options.output.write(
    '#include <cstddef>\n'
    '\n'
    'extern const unsigned char {}[] = {{'.format(array))

  index = 0
  for line in options.input:
    for c in line:

      if index % 12 == 0:
        options.output.write('\n ')

      options.output.write(' 0x{:02x},'.format(ord(c)))

      index += 1

  options.output.write(
    '\n'
    '}};\n'
    'extern const size_t {} = sizeof({}) / sizeof({}[0]);\n'
    .format(size, array, array))

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv))
