#!/usr/bin/env python3

"""
test that rumur-run can check a basic model
"""

import os
import subprocess as sp
import sys

RUMUR_RUN = os.path.join(os.path.dirname(__file__), "../rumur/src/rumur-run")

MODEL = """
var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
"""

def main():
  p = sp.Popen(["python3", RUMUR_RUN], stdin=sp.PIPE)
  p.communicate(MODEL.encode("utf-8", "replace"))
  return p.returncode

if __name__ == "__main__":
  sys.exit(main())
