#!/usr/bin/env python3

import json, os, re, shutil, subprocess, sys, tempfile, unittest

RUMUR_BIN = os.path.abspath('rumur/rumur')
CC = subprocess.check_output(['which', 'cc'], universal_newlines=True).strip()

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

def test_template(self, manifest):
  with open(manifest, 'rt') as f:
    data = json.load(f)

  model = os.path.join(os.path.dirname(manifest), data['model'])

  with TemporaryDirectory() as tmp:

    model_c = os.path.join(tmp, 'model.c')
    subprocess.check_call([RUMUR_BIN, '--output', model_c, model])

    if data.get('compile', True):

      cflags = data.get('cflags', ['-std=c11']) + data.get('extra_cflags', [])

      model_bin = os.path.join(tmp, 'model.bin')
      subprocess.check_call([CC] + cflags + ['-o', model_bin, model_c])

      if data.get('run', True):
        subprocess.check_call([model_bin])

def main(argv):

  if not os.path.isfile(RUMUR_BIN):
    sys.stderr.write('{} not found\n'.format(RUMUR_BIN))
    return -1

  # Find test cases in subdirectories.

  root = os.path.dirname(os.path.abspath(__file__))
  for d in (os.path.join(root, d) for d in os.listdir(root)):

    if not os.path.isdir(d):
      continue

    for m in (os.path.join(d, m) for m in os.listdir(d)):

      if not os.path.isfile(m) or os.path.splitext(m)[1] != '.json':
        continue

      d_name = os.path.basename(d)
      m_name = os.path.basename(m)

      test_name = re.sub(r'[^\w]', '_', 'test_{}_{}'.format(d_name, m_name))
      if hasattr(Tests, test_name):
        raise Exception('{} collides with an existing test name'.format(m))

      setattr(Tests, test_name,
        lambda self, manifest=m: test_template(self, manifest))

  unittest.main()

if __name__ == '__main__':
  sys.exit(main(sys.argv))
