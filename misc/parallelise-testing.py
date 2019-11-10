#!/usr/bin/env python3

'''
run ../tests/integration-tests.py in parallel, taking advantage of multiple
cores
'''

import multiprocessing
import os
import subprocess
import sys

INTEGRATION_TESTS = os.path.abspath(os.path.join(os.path.dirname(__file__),
  '../tests/integration-tests.py'))

# test classes in ../tests/integration-tests.py that we will run in parallel
TEST_CASES = (
  'ASTDumpTests',
  'RumurCMurphiExamplesTests',
  'RumurOptimisedDebugMultiThreadedTests',
  'RumurOptimisedDebugSingleThreadedTests',
  'RumurOptimisedDebugValgrindMultiThreadedTests',
  'RumurOptimisedDebugValgrindSingleThreadedTests',
  'RumurOptimisedMultiThreadedTests',
  'RumurOptimisedSingleThreadedTests',
  'RumurOptimisedValgrindMultiThreadedTests',
  'RumurOptimisedValgrindSingleThreadedTests',
  'RumurOptimisedXMLMultiThreadedTests',
  'RumurOptimisedXMLSingleThreadedTests',
  'RumurUnoptimisedDebugMultiThreadedTests',
  'RumurUnoptimisedDebugSingleThreadedTests',
  'RumurUnoptimisedDebugValgrindMultiThreadedTests',
  'RumurUnoptimisedDebugValgrindSingleThreadedTests',
  'RumurUnoptimisedMultiThreadedTests',
  'RumurUnoptimisedSingleThreadedTests',
  'RumurUnoptimisedValgrindMultiThreadedTests',
  'RumurUnoptimisedValgrindSingleThreadedTests',
  'RumurUnoptimisedXMLMultiThreadedTests',
  'RumurUnoptimisedXMLSingleThreadedTests',
  'StaticRumurTests',
)

print_lock = multiprocessing.Lock()

def run_test_case(case: str):
  '''
  run a single test class
  '''

  p = subprocess.Popen([INTEGRATION_TESTS] + sys.argv[1:] + [case],
    stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  stdout, stderr = p.communicate()

  return p.returncode, stdout.decode('utf-8', 'replace'), \
    stderr.decode('utf-8', 'replace')

def main():
  ret = 0

  with multiprocessing.Pool(multiprocessing.cpu_count()) as pool:
    for r, stdout, stderr in pool.imap_unordered(run_test_case, TEST_CASES):
      ret |= r
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)

  return ret

if __name__ == '__main__':
  sys.exit(main())
