#!/usr/bin/env python

import codecs
import io
import json
import multiprocessing
import os
import platform
import re
import shutil
import subprocess
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET

RUMUR_BIN = os.environ.get('RUMUR', os.path.abspath('rumur/rumur'))
RUMUR_AST_DUMP_BIN = os.environ.get('RUMUR_AST_DUMP', os.path.abspath(
  'ast-dump/rumur-ast-dump'))
CC = os.environ.get('CC', subprocess.check_output(['which', 'cc'],
  universal_newlines=True).strip())

# let the user define a range of tests to run
try:
  MIN_TEST = int(os.environ['MIN_TEST'])
except:
  MIN_TEST = None
try:
  MAX_TEST = int(os.environ['MAX_TEST'])
except:
  MAX_TEST = None
def in_range(index):
  if MIN_TEST is not None and index < MIN_TEST:
    return False
  if MAX_TEST is not None and index > MAX_TEST:
    return False
  return True

try:
  VALGRIND = subprocess.check_output(['which', 'valgrind'],
    universal_newlines=True).strip()
except subprocess.CalledProcessError:
  VALGRIND = None

def valgrind_wrap(args):
  assert VALGRIND is not None
  return [VALGRIND, '--leak-check=full', '--show-leak-kinds=all',
    '--error-exitcode=42'] + args

# function used by test cases for SMT detection
def smt_args():

  # preference 1: Z3
  has_z3 = True
  try:
    with open(os.devnull, 'wt') as null:
      subprocess.check_call(['which', 'z3'], stdout=null, stderr=null)
  except subprocess.CalledProcessError:
    has_z3 = False

  if has_z3:
    # We set a blank logic here, as Z3 performs best when not given a logic. The
    # BV test cases will still override this.
    return ['--smt-logic', '', '--smt-path', 'z3', '--smt-arg=-smt2',
      '--smt-arg=-in']

  # preference 2: CVC4
  has_cvc4 = True
  try:
    with open(os.devnull, 'wt') as null:
      subprocess.check_call(['which', 'cvc4'], stdout=null, stderr=null)
  except subprocess.CalledProcessError:
    has_cvc4 = False

  if has_cvc4:
    return ['--smt-path', 'cvc4', '--smt-arg=--lang=smt2', '--smt-arg=--rewrite-divk']

  # otherwise, give up
  return []

X86_64 = platform.machine() in ('amd64', 'x86_64')

# whether the current platform has sandboxing support for the verifier
def has_sandbox():

  if platform.system() == 'Darwin':
    return True

  if platform.system() == 'FreeBSD':
    return True

  if platform.system() == 'Linux':
    release = platform.release()
    m = re.match(r'(?P<major>\d+)\.(?P<minor>\d+)\.(?P<patch>\d+)', release)

    assert m is not None, 'unrecognised platform.release() string'

    version = tuple(int(m.group(f)) for f in ('major', 'minor', 'patch'))

    return version >= (3, 5, 0)

  if platform.system() == 'OpenBSD':
    return True

  return False

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

  def _test_lock_freedom(self, args):
    '''
    Test that a compiled verifier does not depend on libatomic.

    Uses of __atomic built-ins and C11 atomics can sometimes cause the compiler
    to emit calls to libatomic instead of inline instructions. This is a problem
    because we use these in the verifier to implement lock-free algorithms,
    while the libatomic implementations take locks, defeating the purpose of
    using them. This test checks that we end up with no libatomic calls in the
    compiled verifier.
    '''

    model = 'var x: boolean; startstate begin x := false; end; rule begin x := !x; end;'

    with TemporaryDirectory() as tmp:

      # write the model to a temporary file
      model_m = os.path.join(tmp, 'model.m')
      with open(model_m, 'wt') as f:
        f.write(model)

      # generate a verifier
      argv = [RUMUR_BIN, '--output', 'model.c', model_m]
      subprocess.check_call(argv, cwd=tmp)

      # compile it to assembly
      argv = [CC, '-O3', '-std=c11'] + args + ['-S', 'model.c']
      subprocess.check_call(argv, cwd=tmp)

      # check for calls to libatomic functions
      model_s = os.path.join(tmp, 'model.s')
      with open(model_s, 'rt') as f:
        data = f.read()

      m = re.search('__atomic_', data)
      self.assertIsNone(m)

  @unittest.skipIf(not X86_64 or MAX_TEST is not None, 'only relevant for x86-64')
  def test_lock_freedom_x86_64(self):
    self._test_lock_freedom(['-mcx16'])

  @unittest.skipIf(not X86_64 or MAX_TEST is not None, 'only relevant for x86-64')
  def test_lock_freedom_i386(self):

    # check that we have the i386 headers installed
    with TemporaryDirectory() as tmp:

      test_c = os.path.join(tmp, 'test.c')
      with open(test_c, 'wt') as out:

        includes = os.path.abspath(os.path.join(os.path.dirname(__file__),
          '../rumur/resources/includes.c'))
        with open(includes, 'rt') as inp:
          out.write(inp.read())

        out.write('\nint main(void) { return 0; }\n')

      try:
        with open(os.devnull, 'wt') as null:
          subprocess.check_call([CC, '-std=c11', '-m32', '-o', os.devnull, test_c],
            stdout=null, stderr=null)
      except subprocess.CalledProcessError:
        raise unittest.SkipTest('32-bit headers unavailable')

    self._test_lock_freedom(['-m32'])

  @unittest.skipIf(X86_64 or MAX_TEST is not None, 'not relevant on x86-64')
  def test_lock_freedom(self):
    self._test_lock_freedom([])

def parse_test_options(model, xml):
  option = {}

  # Check for special lines at the start of the current model overriding the
  # defaults.
  with io.open(model, 'rt', encoding='utf-8') as f:
    for line in f:
      m = re.match(r'\s*--\s*(?P<key>[a-zA-Z_]\w*)\s*:(?P<value>.*)$', line)
      if m is None:
        break
      key = m.group('key')
      value = m.group('value').strip()
      option[key] = eval(value)

  return option

def test_template(self, model, optimised, debug, valgrind, xml, multithreaded):

  # Default options to use for this test.
  option = {
    'rumur_flags':[], # Flags to pass to rumur when generating the checker.
    'rumur_exit_code':0, # Expected exit status of rumur.
    'c_flags':None, # Flags to pass to cc when compiling.
    'ld_flags':None, # Flags to pass to cc last.
    'c_exit_code':0, # Expected exit status of cc.
    'checker_exit_code':0, # Expected exit status of the checker.
    'checker_output':None, # Regex to search checker's stdout against.
    'skip_reason':None, # a reason to skip this test
  }

  option.update(parse_test_options(model, xml))

  if option['skip_reason'] is not None:
    self.skipTest(option['skip_reason'])

  with TemporaryDirectory() as tmp:

    model_c = os.path.join(tmp, 'model.c')
    rumur_flags = ['--output', model_c, model]
    if debug:
      rumur_flags.append('--debug')
    if xml:
      rumur_flags.extend(['--output-format', 'machine-readable'])
    if multithreaded and multiprocessing.cpu_count() == 1:
      # we need to force multiple threads, or Rumur will autodetect --threads 1
      rumur_flags.extend(['--threads', '2'])
    elif not multithreaded:
      rumur_flags.extend(['--threads', '1'])
    args = [RUMUR_BIN] + rumur_flags + option['rumur_flags']
    if valgrind:
      args = valgrind_wrap(args)
    ret, stdout, stderr = run(args)
    if valgrind:
      if ret == 42:
        sys.stdout.write(stdout)
        sys.stderr.write(stderr)
      self.assertNotEqual(ret, 42)
      return
    if ret != option['rumur_exit_code']:
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)
    self.assertEqual(ret, option['rumur_exit_code'])

    # If model generation was supposed to fail, we're done.
    if option['rumur_exit_code'] != 0:
      return

    if option['c_flags'] is None:
      cflags = ['-std=c11', '-Werror=format', '-Werror=sign-compare', '-Werror=type-limits']
      if X86_64:
        cflags.append('-mcx16')
      if optimised:
        cflags.extend(['-O3', '-fwhole-program'])
    else:
      cflags = option['c_flags']

    if option['ld_flags'] is None:
      ldflags = ['-lpthread']
    else:
      ldflags = []

    model_bin = os.path.join(tmp, 'model.bin')
    args = [CC] + cflags + ['-o', model_bin, model_c] + ldflags
    ret, stdout, stderr = run(args)
    if ret != option['c_exit_code']:
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)
    self.assertEqual(ret, option['c_exit_code'])

    # If compilation was supposed to fail, we're done.
    if option['c_exit_code'] != 0:
      return

    ret, stdout, stderr = run([model_bin])
    if ret != option['checker_exit_code']:
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)
    self.assertEqual(ret, option['checker_exit_code'])

    # If the test has a stdout expectation, check that now.
    if option['checker_output'] is not None:
      if option['checker_output'].search(stdout) is None:
        sys.stdout.write(stdout)
        sys.stderr.write(stderr)
      self.assertIsNotNone(option['checker_output'].search(stdout))

    # coarse grained check for whether the model contains a 'put' statement that
    # could screw up XML validation
    with io.open(model, 'rt', encoding='utf-8') as f:
      has_put = re.search(r'\bput\b', f.read()) is not None

    if xml and not has_put:
      # See if we have xmllint
      ret, _, _ = run(['which', 'xmllint'])
      if ret != 0:
        self.skipTest('xmllint not available for validation')

      # Validate the XML
      rng = os.path.abspath(os.path.join(os.path.dirname(__file__),
        '..', 'misc', 'verifier.rng'))
      output_xml = os.path.join(tmp, 'output.xml')
      with open(output_xml, 'wt') as f:
        f.write(stdout)
        f.flush()
      ret, stdout, stderr = run(['xmllint', '--relaxng', rng, '--noout',
        output_xml])
      if ret != 0:
        sys.stderr.write('Failed to XML-validate machine readable output\n')
        sys.stdout.write(stdout)
        sys.stderr.write(stderr)
      self.assertEqual(ret, 0)

def test_ast_dumper_template(self, model, valgrind):

  with TemporaryDirectory() as tmp:

    model_xml = os.path.join(tmp, 'model.xml')
    ad_flags = ['--output', model_xml, model]
    args = [RUMUR_AST_DUMP_BIN] + ad_flags
    if valgrind:
      args = valgrind_wrap(args)
    ret, stdout, stderr = run(args)
    if valgrind:
      if ret == 42:
        sys.stdout.write(stdout)
        sys.stderr.write(stderr)
      self.assertNotEqual(ret, 42)
      # Remainder of the test is unnecessary, because we will already test this
      # in the version of this test when valgrind=False.
      return
    if ret != 0:
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)
    self.assertEqual(ret, 0)

    # See if we have xmllint
    ret, _, _ = run(['which', 'xmllint'])
    if ret != 0:
      self.skipTest('xmllint not available for validation')

    # Validate the XML
    rng = os.path.abspath(os.path.join(os.path.dirname(__file__),
      '..', 'misc', 'ast-dump.rng'))
    ret, stdout, stderr = run(['xmllint', '--relaxng', rng, '--noout',
      model_xml])
    if ret != 0:
      with open(model_xml, 'rt') as f:
        sys.stderr.write('Failed to validate:\n{}\n'.format(f.read()))
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)
    self.assertEqual(ret, 0)

def test_cmurphi_example_template(self, model, outcome, rules_fired=None,
    states=None):

  with TemporaryDirectory() as tmp:

    model_c = os.path.join(tmp, 'model.c')
    ret, stdout, stderr = run([RUMUR_BIN, '--output-format', 'machine-readable',
      '--output', model_c, model])
    if ret != 0:
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)
    self.assertEqual(ret, 0)

    cflags = ['-std=c11', '-O3', '-fwhole-program', '-Werror=format']
    if X86_64:
      cflags.append('-mcx16')
    model_bin = os.path.join(tmp, 'model.bin')
    ret, stdout, stderr = run([CC] + cflags + ['-o', model_bin, model_c,
      '-lpthread'])
    if ret != 0:
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)
    self.assertEqual(ret, 0)

    ret, stdout, stderr = run([model_bin])
    if (ret == 0) != outcome:
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)
    self.assertEqual(ret == 0, outcome)

    # parse the XML output if we're expecting a particular result
    if rules_fired is not None or states is not None:
      xml = ET.fromstring(stdout)

      # check we got the expected number of rules fired
      if rules_fired is not None:
        r = int(xml.find('./summary').get('rules_fired'))
        self.assertEqual(r, rules_fired)

      # check we got the expected number of states explored
      if states is not None:
        s = int(xml.find('./summary').get('states'))
        self.assertEqual(s, states)

def test_ast_dumper_cmurphi_example_template(self, model):

  with TemporaryDirectory() as tmp:

    model_xml = os.path.join(tmp, 'model.xml')
    ad_flags = ['--output', model_xml, model]
    args = [RUMUR_AST_DUMP_BIN] + ad_flags
    ret, stdout, stderr = run(args)
    if ret != 0:
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)
    self.assertEqual(ret, 0)

    # See if we have xmllint
    ret, _, _ = run(['which', 'xmllint'])
    if ret != 0:
      self.skipTest('xmllint not available for validation')

    # Validate the XML
    ret, stdout, stderr = run(['xmllint', '--noout', model_xml])
    if ret != 0:
      with open(model_xml, 'rt') as f:
        sys.stderr.write('Failed to validate:\n{}\n'.format(f.read()))
      sys.stdout.write(stdout)
      sys.stderr.write(stderr)
    self.assertEqual(ret, 0)

def main():

  # setup stdout/stderr to make encoding errors non-fatal
  try:
    sys.stdout = codecs.getwriter('utf-8')(sys.stdout.buffer, 'replace')
  except:
    sys.stdout = codecs.getwriter('utf-8')(sys.stdout, 'replace')
  try:
    sys.stderr = codecs.getwriter('utf-8')(sys.stderr.buffer, 'replace')
  except:
    sys.stderr = codecs.getwriter('utf-8')(sys.stderr, 'replace')

  try:
    with open(os.devnull, 'wt') as null:
      subprocess.check_call(['which', RUMUR_BIN], stdout=null, stderr=null)
  except subprocess.CalledProcessError:
    sys.stderr.write('{} not found\n'.format(RUMUR_BIN))
    return -1

  index = 0

  # Find test cases in subdirectories.

  root = os.path.dirname(os.path.abspath(__file__))
  for m in (os.path.join(root, m) for m in os.listdir(root)):

    if not os.path.isfile(m) or os.path.splitext(m)[1] != '.m':
      continue

    m_name = os.path.basename(m)

    for optimised in (False, True):
      for debug in (False, True):
        for valgrind in (False, True):
          for xml in (False, True):
            for multithreaded in (False, True):

              # Don't test machine-readable output for a debug or Valgrind run,
              # as this messes up XML.
              if xml and (debug or valgrind):
                continue

              if valgrind and VALGRIND is None:
                # Valgrind unavailable
                continue

              test_name = re.sub(r'[^\w]', '_', 'test_{}{}{}{}{}{}'.format(
                'debug_' if debug else '',
                'optimised_' if optimised else 'unoptimised_',
                'valgrind_' if valgrind else '',
                'xml_' if xml else '',
                'multithreaded_' if multithreaded else 'singlethreaded_',
                m_name))

              if hasattr(Tests, test_name):
                raise Exception('{} collides with an existing test name'.format(m))

              if in_range(index):
                setattr(Tests, test_name,
                  lambda self, model=m, o=optimised, d=debug, v=valgrind, x=xml,
                    m=multithreaded:
                      test_template(self, model, o, d, v, x, m))
              index += 1

    for valgrind in (False, True):

      if valgrind and VALGRIND is None:
        # Valgrind unavailable.
        continue

      # Now we want to add an AST dumper test, but skip this if the input model is
      # expected to fail.
      option = { 'rumur_exit_code':0 }
      option.update(parse_test_options(m, False)) # <- False used as dummy arg
      if not valgrind and option['rumur_exit_code'] != 0:
        continue

      test_name = re.sub(r'[^\w]', '_', 'test_ast_dumper_{}{}'.format(
        'valgrind_' if valgrind else '', m_name))

      if hasattr(Tests, test_name):
        raise Exception('{} collides with an existing test name'.format(m))

      if in_range(index):
        setattr(Tests, test_name,
          lambda self, model=m, v=valgrind: test_ast_dumper_template(self, model, v))
      index += 1

  # If the user has told us where a copy of the CMurphi source is, test some of
  # the example models distributed with CMurphi.
  CMURPHI_DIR = os.environ.get('CMURPHI_DIR')
  if CMURPHI_DIR is not None:

    models = (
      # (Model path,           expected to pass?  expected rules  expected states)
      ('ex/mux/2_peterson.m',  True,              26,             13),
      ('ex/mux/dek.m',         True,              200,            100),
      ('ex/mux/mcslock1.m',    True,              None,           None),
      ('ex/mux/mcslock2.m',    True,              None,           None),
      ('ex/mux/n_peterson.m',  True,              None,           None),
      ('ex/others/abp.m',      True,              176,            80),
      ('ex/others/arbiter.m',  False,             None,           None),
      ('ex/others/dp4.m',      True,              672,            112),
      ('ex/others/dpnew.m',    False,             None,           None),
      ('ex/sym/mcslock1.m',    True,              None,           None),
      ('ex/sym/mcslock2.m',    True,              None,           None),
      ('ex/sym/n_peterson.m',  True,              None,           None),
      ('ex/tmp/scalarset.m',   False,             None,           None),
      ('ex/toy/down.m',        False,             None,           None),
      ('ex/toy/lin.m',         False,             None,           None),
      ('ex/toy/pingpong.m',    True,              6,              4),
      ('ex/toy/sets.m',        False,             None,           None),
      ('ex/toy/sort5.m',       False,             None,           None),
    )

    for path, outcome, rules, states in models:
      fullpath = os.path.abspath(os.path.join(CMURPHI_DIR, path))

      test_name = re.sub(r'[^\w]', '_', 'test_cmurphi_example_{}'.format(path))

      if hasattr(Tests, test_name):
        raise Exception('{} collides with an existing test name'.format(path))

      if in_range(index):
        setattr(Tests, test_name,
          lambda self, model=fullpath, outcome=outcome, rules=rules, states=states:
            test_cmurphi_example_template(self, model, outcome, rules, states))
      index += 1

      test_name = re.sub(r'[^\w]', '_', 'test_ast_dumper_cmurphi_example_{}'
        .format(path))

      if hasattr(Tests, test_name):
        raise Exception('{} collides with an existing test name'.format(path))

      if in_range(index):
        setattr(Tests, test_name,
          lambda self, model=fullpath:
            test_ast_dumper_cmurphi_example_template(self, model))
      index += 1

  unittest.main()

if __name__ == '__main__':
  sys.exit(main())
