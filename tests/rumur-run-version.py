#!/usr/bin/env python3

"""
basic test that rumur-run can execute successfully
"""

import os
import subprocess
import sys

RUMUR_RUN = os.path.join(os.path.dirname(__file__), "../rumur/src/rumur-run")

def main():
  subprocess.check_call(["python3", RUMUR_RUN, "--version"])

if __name__ == "__main__":
  sys.exit(main())
