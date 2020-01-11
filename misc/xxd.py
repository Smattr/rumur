#!/usr/bin/env python3

import argparse
import re
import sys

def main(args: [str]) -> int:

  # parse command line arguments
  parser = argparse.ArgumentParser(
    description='convert file contents to a C++ string')
  parser.add_argument('input', type=argparse.FileType('rb'), help='input file')
  parser.add_argument('output', type=argparse.FileType('wt'),
    help='output file')
  options = parser.parse_args(args[1:])

  array = re.sub(r'[^\w\d]', '_', options.input.name)
  size = f'{array}_len'

  options.output.write(
     '#include <cstddef>\n'
     '\n'
    f'extern const unsigned char {array}[] = {{')

  index = 0
  while True:

    c = options.input.read(1)
    if c == b'':
      break

    if index % 12 == 0:
      options.output.write('\n ')

    options.output.write(f' 0x{int.from_bytes(c, byteorder="little"):02x},')

    index += 1

  options.output.write(
     '\n'
     '};\n'
    f'extern const size_t {size} = sizeof({array}) / sizeof({array}[0]);\n')

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv))
