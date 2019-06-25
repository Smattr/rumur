#!/usr/bin/env python

'''
Toy SMT solver for use in the test suite. This script pretends to be an SMT
solver but is capable of answering only simply queries containing the
contradiction '(assert (not (= v v)))'. For anything else it says 'unknown'.
'''

import re
import sys

query = sys.stdin.read()

assert '(check-sat)' in query, 'unexpected query input'

if re.search(r'\(\s*assert\s*\(\s*not\s*\(\s*=\s+(?P<var>[a-zA-Z_]\w*)'
    r'\s+(?P=var)\s*\)\s*\)\s*\)', query) is not None:
  sys.stdout.write('unsat\n')
else:
  sys.stdout.write('unknown\n')

sys.exit(0)
