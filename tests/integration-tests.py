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

  with TemporaryDirectory() as tmp:

    model_c = os.path.join(tmp, 'model.c')
    ret, stdout, stderr = run([RUMUR_BIN, '--output', model_c, model])
    if ret != 0:
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)
    self.assertEqual(ret, 0)

    cflags = ['-std=c11']
    if optimised:
      cflags.extend(['-O3', '-fwhole-program'])

    model_bin = os.path.join(tmp, 'model.bin')
    ret, stdout, stderr = run([CC] + cflags + ['-o', model_bin, model_c])
    if ret != 0:
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)
    self.assertEqual(ret, 0)

    ret, stdout, stderr = run([model_bin])
    if ret != 0:
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)
    self.assertEqual(ret, 0)

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
