#!/usr/bin/env python3

import argparse
import re
import sys
import textwrap

def main(args):

  # parse command line arguments
  parser = argparse.ArgumentParser(
    description="convert file contents to a C++ string")
  parser.add_argument("input", type=argparse.FileType("rb"), help="input file")
  parser.add_argument("output", type=argparse.FileType("wt"),
    help="output file")
  options = parser.parse_args(args[1:])

  array = re.sub(r"[^\w\d]", "_", options.input.name)
  size = "{}_len".format(array)

  options.output.write(textwrap.dedent("""\
  #include <cstddef>

  extern const unsigned char {}[] = {{""".format(array)))

  index = 0
  while True:

    c = options.input.read(1)
    if c == b"":
      break

    if index % 12 == 0:
      options.output.write("\n ")

    options.output.write(
      " 0x{:02x},".format(int.from_bytes(c, byteorder="little")))

    index += 1

  options.output.write(textwrap.dedent("""\

  }};
  extern const size_t {} = sizeof({}) / sizeof({}[0]);
  """.format(size, array, array)))

  return 0

if __name__ == "__main__":
  sys.exit(main(sys.argv))
