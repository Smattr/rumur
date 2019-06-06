#!/usr/bin/env python

'''
Wrapper for fuzzing Rumur with AFL using the existing test suite.
'''

import os
import subprocess
import sys
import time

def main(argv):

  timeout = None
  if len(argv) > 1:
    timeout = int(argv[1])

  env = os.environ.copy()
  env['PATH'] = '{}:{}'.format(env['PATH'], os.path.abspath('rumur'))

  # note we need to up AFL's default memory limit (50MB) to support running CC
  sys.stdout.write(' Running AFL...\n')
  testdir = os.path.abspath(os.path.dirname(__file__))
  p = subprocess.Popen(['afl-fuzz', '-m', '8192', '-i', testdir, '-o', 'findings_dir',
    'afl-harness', '@@'], env=env)

  start = time.time()
  counter = 0
  while timeout is None or time.time() - start < timeout:

    if p.poll() is not None:
      # AFL has finished
      break

    # periodically output something to pacify Travis CI's timeout monitor
    if counter % 60 == 0:
      sys.stdout.write('\x00')
      sys.stdout.flush()

    time.sleep(1)
    counter += 1

  if p.poll() is None:
    # AFL is still running
    p.terminate()
    p.wait()

  found = False
  crashes = 'findings_dir/crashes'
  if os.path.exists(crashes):
    for i in os.listdir(crashes):
      found = True
      path = os.path.join(crashes, i)
      sys.stdout.write('crash in {}:\n'.format(path))
      subprocess.call(['cat', path])
      sys.stdout.write('\n{}\n'.format('-' * 80))

  return -1 if found else 0

if __name__ == '__main__':
  sys.exit(main(sys.argv))
