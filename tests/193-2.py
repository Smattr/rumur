#!/usr/bin/env python3

"""
This is a variant of 193.m that tests that we descend into TypeExprIDs when
reordering record fields. It is hard to trigger a behaviour divergence that is
exposed during checking, so instead we check the --debug output of rumur.

See also https://github.com/Smattr/rumur/issues/193.
"""

import os
import subprocess as sp
import sys

# a model containing a TypeExprID, itself referring to a record whose fields
# will be reordered
MODEL = """
type
  t1: scalarset(4);

  t2: record
    a: boolean;
    b: t1;
    c: boolean;
  end;

var x: t2;

startstate begin
end

rule begin
end
"""

def main():

  # run Rumur in debug mode
  output = sp.check_output(["rumur", "--output", os.devnull, "--debug"],
    stderr=sp.STDOUT, input=MODEL.encode("utf-8", "replace"))
  output = output.decode("utf-8", "replace")

  # we should see the fields be reordered once due to encountering the original
  # TypeDecl (t2) and once due to the TypeExprID (the type of x)
  assert output.count("sorted fields {a, b, c} -> {a, c, b}") == 2

  return 0

if __name__ == "__main__":
  sys.exit(main())
