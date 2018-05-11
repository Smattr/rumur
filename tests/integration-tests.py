#!/usr/bin/env python3

import json, pathlib, re, shutil, subprocess, sys, tempfile, unittest

RUMUR_BIN = pathlib.Path('rumur/rumur').resolve()
CC = pathlib.Path(subprocess.check_output(['which', 'cc'],
  universal_newlines=True).strip())

class TemporaryDirectory(object):

  def __init__(self):
    self.tmp = None

  def __enter__(self):
    self.tmp = tempfile.mkdtemp()
    return pathlib.Path(self.tmp)

  def __exit__(self, *_):
    if self.tmp is not None:
      shutil.rmtree(self.tmp)

class Tests(unittest.TestCase):
  pass

def test_template(self, manifest):
  with open(manifest, 'rt') as f:
    data = json.load(f)

  model = manifest.parent / data['model']

  with TemporaryDirectory() as tmp:

    model_c = tmp / 'model.c'
    subprocess.check_call([RUMUR_BIN, '--output', model_c, model])

    if data.get('compile', True):

      cflags = data.get('cflags', ['-std=c11']) + data.get('extra_cflags', [])

      model_bin = tmp / 'model.bin'
      subprocess.check_call([CC] + cflags + ['-o', model_bin, model_c])

      if data.get('run', True):
        subprocess.check_call([model_bin])

def main(argv):

  if not RUMUR_BIN.is_file():
    sys.stderr.write('{} not found\n'.format(RUMUR_BIN))
    return -1

  # Find test cases in subdirectories.

  root = pathlib.Path(__file__).resolve().parent
  for d in (d for d in root.iterdir() if d.is_dir()):

    for m in (m for m in d.iterdir() if m.is_file() and m.suffix == '.json'):

      test_name = re.sub(r'[^\w]', '_', 'test_{}_{}'.format(d.name, m.name))
      if hasattr(Tests, test_name):
        raise Exception('{} collides with an existing test name'.format(m))

      setattr(Tests, test_name,
        lambda self, manifest=m: test_template(self, manifest))

  unittest.main()

if __name__ == '__main__':
  sys.exit(main(sys.argv))
