#!/usr/bin/env python3

"""
Version of lock-freedom.py for ARM64, which needs -march=native
"""

import os
import subprocess
import sys

# this test is only relevant on ≥ARMv8.1a
CC = os.environ.get("CC", "cc")
argv = [CC, "-std=c11", "-march=armv8.1-a", "-x", "c", "-", "-o", os.devnull]
p = subprocess.Popen(argv, stdin=subprocess.PIPE, stderr=subprocess.DEVNULL,
  universal_newlines=True)
p.communicate("int main(void) { return 0; }")
if p.returncode != 0:
  print("only relevant for ≥ARMv8.1a machines")
  sys.exit(125)

# call the generic lock-freedom.py with some customisation
lock_freedom_py = os.path.join(os.path.dirname(__file__), "lock-freedom.py")
sys.exit(subprocess.call(["python3", lock_freedom_py, "-march=armv8.1-a"]))
