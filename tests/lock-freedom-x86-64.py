#!/usr/bin/env python3

"""
x86-64 version of lock-freedom.py, which needs -mcx16
"""

import os
import platform
import sys
import subprocess

# this test is only relevant on x86-64
if platform.machine() not in ("amd64", "x86_64"):
  print("only relevant for x86-64 machines")
  sys.exit(125)

# call the generic lock-freedom.py with some customisation
lock_freedom_py = os.path.join(os.path.dirname(__file__), "lock-freedom.py")
sys.exit(subprocess.call(["python3", lock_freedom_py, "-mcx16"]))
