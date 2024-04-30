#!/usr/bin/env python3

"""
Version of lock-freedom.py for 32-bit compilation on x86-64
"""

import os
import platform
import sys
import subprocess
import textwrap

# this test is only relevant on x86-64
if platform.machine() not in ("amd64", "x86_64"):
  print("only relevant for x86-64 machines")
  sys.exit(125)

# check that we have a multilib compiler capable of targeting i386
CC = os.environ.get("CC", "cc")
argv = [CC, "-std=c11", "-m32", "-o", os.devnull, "-x", "c", "-"]
program = textwrap.dedent("""\
#include <stdio.h>
int main(void) {
  printf("hello world\\n");
  return 0;
}
""")
p = subprocess.Popen(argv, stdin=subprocess.PIPE, stderr=subprocess.DEVNULL)
p.communicate(program.encode("utf-8", "replace"))
if p.returncode != 0:
  print("compiler cannot target 32-bit code")
  sys.exit(125)

# call the generic lock-freedom.py with some customisation
lock_freedom_py = os.path.join(os.path.dirname(__file__), "lock-freedom.py")
sys.exit(subprocess.call(["python3", lock_freedom_py, "-m32"]))
