#!/usr/bin/env python3

"""
Test that a compiled verifier does not depend on libatomic.

Uses of __atomic built-ins and C11 atomics can sometimes cause the compiler to
emit calls to libatomic instead of inline instructions. This is a problem
because we use these in the verifier to implement lock-free algorithms, while
the libatomic implementations take locks, defeating the purpose of using them.
This test checks that we end up with no libatomic calls in the compiled
verifier.
"""

import os
import platform
import sys
import subprocess

# generate a checker for a simple model
model = "var x: boolean; startstate begin x := false; end; rule begin x := !x; end;"
argv = ["rumur", "--output", "/dev/stdout"]
p = subprocess.Popen(argv, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
model_c, _ = p.communicate(model.encode("utf-8", "replace"))
if p.returncode != 0:
  print("call to rumur failed")
  sys.exit(1)

# compile it to assembly
CC = os.environ.get("CC", "cc")
argv = [CC, "-O3", "-std=c11", "-x", "c", "-", "-S", "-o", "/dev/stdout"] \
  + sys.argv[1:]
p = subprocess.Popen(argv, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
model_s, _ = p.communicate(model_c)
if p.returncode != 0:
  print("compilation failed")
  sys.exit(1)

# check for calls to libatomic functions
if "__atomic_" in model_s.decode("utf-8", "replace"):
  print("libatomic calls in generated code were not optimised out:\n{}".format(
    model_s.decode("utf-8", "replace")))
  sys.exit(-1)

# pass
sys.exit(0)
