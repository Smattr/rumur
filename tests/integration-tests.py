#!/usr/bin/env python3

import json, os, re, shutil, subprocess, sys, tempfile, unittest

RUMUR_BIN = os.path.abspath('rumur/rumur')
CC = subprocess.check_output(['which', 'cc'], universal_newlines=True).strip()

def run(args):
  p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  stdout, stderr = p.communicate()
  return p.returncode, stdout.decode('utf-8', 'replace'), stderr.decode('utf-8', 'replace')

class TemporaryDirectory(object):

  def __init__(self):
    self.tmp = None

  def __enter__(self):
    self.tmp = tempfile.mkdtemp()
    return self.tmp

  def __exit__(self, *_):
    if self.tmp is not None:
      shutil.rmtree(self.tmp)

class Tests(unittest.TestCase):
  pass

def test_template(self, model, optimised):

  # Default options to use for this test.
  option = {
    'rumur_flags':[], # Flags to pass to rumur when generating the checker.
    'rumur_exit_code':0, # Expected exit status of rumur.
    'c_flags':None, # Flags to pass to cc when compiling.
    'c_exit_code':0, # Expected exit status of cc.
    'checker_exit_code':0, # Expected exit status of the checker.
  }

  # Check for special lines at the start of the current model overriding the
  # defaults.
  with open(model, 'rt') as f:
    for line in f:
      m = re.match(r'\s*--\s*(?P<key>[a-zA-Z_]\w*)\s*:(?P<value>.*)$', line)
      if m is None:
        break
      key = m.group('key')
      value = m.group('value').strip()
      option[key] = eval(value)

  with TemporaryDirectory() as tmp:

    model_c = os.path.join(tmp, 'model.c')
    ret, stdout, stderr = run([RUMUR_BIN, '--output', model_c, model] +
      option['rumur_flags'])
    if ret != 0:
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)
    self.assertEqual(ret, option['rumur_exit_code'])

    # If model generation was supposed to fail, we're done.
    if option['rumur_exit_code'] != 0:
      return

    if option['c_flags'] is None:
      cflags = ['-std=c11']
      if optimised:
        cflags.extend(['-O3', '-fwhole-program'])
    else:
      cflags = option['c_flags']

    model_bin = os.path.join(tmp, 'model.bin')
    ret, stdout, stderr = run([CC] + cflags + ['-o', model_bin, model_c])
    if ret != 0:
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)
    self.assertEqual(ret, option['c_exit_code'])

    # If compilation was supposed to fail, we're done.
    if option['c_exit_code'] != 0:
      return

    ret, stdout, stderr = run([model_bin])
    if ret != 0:
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)
    self.assertEqual(ret, option['checker_exit_code'])

def main(argv):

  if not os.path.isfile(RUMUR_BIN):
    sys.stderr.write('{} not found\n'.format(RUMUR_BIN))
    return -1

  # Find test cases in subdirectories.

  root = os.path.dirname(os.path.abspath(__file__))
  for m in (os.path.join(root, m) for m in os.listdir(root)):

    if not os.path.isfile(m) or os.path.splitext(m)[1] != '.m':
      continue

    m_name = os.path.basename(m)

    test_name = re.sub(r'[^\w]', '_', 'test_unoptimised_{}'.format(m_name))
    if hasattr(Tests, test_name):
      raise Exception('{} collides with an existing test name'.format(m))

    setattr(Tests, test_name,
      lambda self, model=m: test_template(self, model, False))

    test_name = re.sub(r'[^\w]', '_', 'test_optimised_{}'.format(m_name))

    setattr(Tests, test_name,
      lambda self, model=m: test_template(self, model, True))

  unittest.main()

if __name__ == '__main__':
  sys.exit(main(sys.argv))
