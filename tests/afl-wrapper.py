#!/usr/bin/env python3

'''
Wrapper for compiling Rumur with instrumentation for American Fuzzy Lop (AFL)
and then fuzzing using the existing test suite.
'''

import os, platform, re, shutil, subprocess, sys, tempfile

def which(cmd: str):
  try:
    with open(os.devnull, 'wt') as null:
      return subprocess.check_output(['which', cmd], stderr=null).strip()
  except:
    return None

# Allow environment variables to override any of the tools we need

AFL_FUZZ = which(os.environ.get('AFL_FUZZ', 'afl-fuzz'))
CMAKE    = which(os.environ.get('CMAKE', 'cmake'))
CXX      = which(os.environ.get('CXX',
  'afl-clang++' if platform.system() == 'Darwin' else 'afl-g++'))
MAKE     = which(os.environ.get('MAKE', 'make'))
RUMUR_ROOT = os.path.abspath(os.environ.get('RUMUR_ROOT',
  os.path.join(os.path.dirname(__file__), '..')))

def main(argv: [str]):

  tmp = tempfile.mkdtemp()
  sys.stdout.write('Working in {}...\n'.format(tmp))

  if CMAKE is None:
    sys.stderr.write('cmake not found\n')
    return -1

  if CXX is None:
    sys.stderr.write('AFL c++ wrapper not found\n')
    return -1

  sys.stdout.write(' Configuring...\n')
  p = subprocess.Popen([CMAKE, '-G', 'Unix Makefiles', RUMUR_ROOT], cwd=tmp,
    env={**os.environ, **{'CXX':CXX}})
  p.communicate()
  if p.returncode != 0:
    return p.returncode

  if MAKE is None:
    sys.stderr.write('make not found\n')
    return -1

  sys.stdout.write(' Building...\n')
  p = subprocess.Popen([MAKE], cwd=tmp)
  p.communicate()
  if p.returncode != 0:
    return p.returncode

  # Copy into the test case directory all tests that are not expected to fail
  sys.stdout.write(' Populating {}/testcase_dir...\n'.format(tmp))
  test_root = os.path.dirname(__file__)
  testcase_dir = os.path.join(tmp, 'testcase_dir')
  os.mkdir(testcase_dir)
  for i in os.listdir(test_root):

    if os.path.splitext(i)[-1].lower() != '.m':
      continue

    src = os.path.join(test_root, i)
    dst = os.path.join(testcase_dir, i)

    shutil.copyfile(src, dst)

  if AFL_FUZZ is None:
    sys.stderr.write('afl-fuzz not found\n')
    return -1

  sys.stdout.write(' Running AFL...\n')
  os.chdir(tmp)
  os.execl(AFL_FUZZ, AFL_FUZZ, '-i', 'testcase_dir', '-o', 'findings_dir',
    './rumur/rumur', '--output', os.devnull, '@@')

if __name__ == '__main__':
  sys.exit(main(sys.argv))
