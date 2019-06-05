#!/usr/bin/env python

'''
Wrapper for compiling Rumur with instrumentation for American Fuzzy Lop (AFL)
and then fuzzing using the existing test suite.
'''

import os
import platform
import shutil
import subprocess
import sys
import tempfile
import time

def which(cmd):
  try:
    with open(os.devnull, 'wt') as null:
      return subprocess.check_output(['which', cmd], stderr=null).strip()
  except:
    return None

def find(env_var, cmd):
  return which(os.environ.get(env_var, cmd))

# Allow environment variables to override any of the tools we need

AFL_FUZZ = find('AFL_FUZZ', 'afl-fuzz')

def main(argv):

  timeout = None
  if len(argv) > 1:
    timeout = int(argv[1])

  tmp = tempfile.mkdtemp()
  sys.stdout.write('Working in {}...\n'.format(tmp))

  # Copy into the test case directory all active tests
  sys.stdout.write(' Populating {}/testcase_dir...\n'.format(tmp))
  test_root = os.path.abspath(os.path.dirname(__file__))
  testcase_dir = os.path.join(tmp, 'testcase_dir')
  os.mkdir(testcase_dir)
  for i in os.listdir(test_root):

    if os.path.splitext(i)[-1].lower() != '.m':
      continue

    src = os.path.join(test_root, i)
    dst = os.path.join(testcase_dir, i)

    shutil.copyfile(src, dst)

  env = os.environ.copy()
  env['PATH'] = '{}:{}'.format(env['PATH'], os.path.abspath('rumur'))

  # note we need to up AFL's default memory limit (50MB) to support running CC
  sys.stdout.write(' Running AFL...\n')
  p = subprocess.Popen([AFL_FUZZ, '-m', '8192', '-i', 'testcase_dir', '-o', 'findings_dir',
    'afl-harness', '@@'], cwd=tmp, env=env)

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
  crashes = os.path.join(tmp, 'findings_dir/crashes')
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
