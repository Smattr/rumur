#!/usr/bin/env python3

import abc
import argparse
import codecs
import enum
import itertools
import multiprocessing
import os
import platform
import re
import shutil
import subprocess as sp
import sys
import tempfile
from typing import Any, Dict, Optional, Tuple

CPUS = multiprocessing.cpu_count()

STDOUT_ISATTY = os.isatty(sys.stdout.fileno())

GREEN  = '\033[32m' if STDOUT_ISATTY else ''
RED    = '\033[31m' if STDOUT_ISATTY else ''
YELLOW = '\033[33m' if STDOUT_ISATTY else ''
RESET  = '\033[0m'  if STDOUT_ISATTY else ''

# configuration (will be populated from config/)
config = None

def enc(s): return s.encode('utf-8', 'replace')
def dec(s): return s.decode('utf-8', 'replace')

# let the user define a range of tests to run
try:
  MIN_TEST = int(os.environ['MIN_TEST'])
except:
  MIN_TEST = None
try:
  MAX_TEST = int(os.environ['MAX_TEST'])
except:
  MAX_TEST = None
def in_range(index: int) -> bool:
  if MIN_TEST is not None and index < MIN_TEST:
    return False
  if MAX_TEST is not None and index > MAX_TEST:
    return False
  return True

print_lock = multiprocessing.Lock()

def pr(s: str) -> None:
  print_lock.acquire()
  sys.stdout.write(s)
  sys.stdout.flush()
  print_lock.release()

def run(args: [str], stdin: Optional[str] = None) -> Tuple[int, str, str]:
  if stdin is not None:
    stdin = enc(stdin)
  env = {k: v for k, v in os.environ.items()}
  env.update({k: str(v) for k, v in config.items()})
  p = sp.run(args, stdout=sp.PIPE, stderr=sp.PIPE, input=stdin, env=env)
  return p.returncode, dec(p.stdout), dec(p.stderr)

VERIFIER_RNG = os.path.abspath(os.path.join(os.path.dirname(__file__),
  '../misc/verifier.rng'))
MURPHI2XML_RNG = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'misc',
      'murphi2xml.rng'))

class TemporaryDirectory(object):
  'an mkdtemp() that cleans up after itself'
  def __init__(self):
    self.tmp = None
  def __enter__(self) -> str:
    self.tmp = tempfile.mkdtemp()
    return self.tmp
  def __exit__(self, *_):
    if self.tmp is not None:
      shutil.rmtree(self.tmp)

class Result(abc.ABC): pass
class Skip(Result):
  def __init__(self, reason: str): self.reason = reason
class Fail(Result):
  def __init__(self, output: str): self.output = output

class Test(abc.ABC):
  @abc.abstractmethod
  def description(self) -> str: raise NotImplementedError
  @abc.abstractmethod
  def run(self) -> Optional[Result]: raise NotImplementedError

# recogniser for a '-- rumur_flags: ...' etc line
TWEAK_LINE = re.compile(r'\s*--\s*(?P<key>[a-zA-Z_]\w*)\s*:(?P<value>.*)$')

class Tweakable(Test):
  'a test case that can take extra customisation via comment lines'
  def __init__(self):
    # default options
    self.rumur_flags = []
    self.rumur_exit_code = 0
    self.checker_exit_code = 0
    self.checker_output = None
    self.skip_reason = None
  def apply_options(self, model: str) -> None:
    'check for special lines at the start of current model overriding defaults'
    with open(model, 'rt', encoding='utf-8') as f:
      for line in f:
        m = TWEAK_LINE.match(line)
        if m is None:
          break
        key = m.group('key')
        value = m.group('value').strip()
        setattr(self, key, eval(value))

class Model(Tweakable):
  def __init__(self, model: str, debug: bool, optimised: bool, \
      multithreaded: bool, xml: bool):
    super().__init__()
    self.model = model
    self.debug = debug
    self.optimised = optimised
    self.multithreaded = multithreaded
    self.xml = xml
    self.valgrind = config['HAS_VALGRIND']
  def description(self) -> str:
    return f'{"D" if self.debug              else " "}' \
           f'{"O" if self.optimised          else " "}' \
           f'{"M" if self.multithreaded      else " "}' \
           f'{"X" if self.xml                else " "}' \
           f'{"V" if config["HAS_VALGRIND"] else " "}' \
           f' {os.path.basename(self.model)}'
  def run(self) -> Result:

    self.apply_options(self.model)

    if self.skip_reason is not None: return Skip(self.skip_reason)

    # build up arguments to call rumur
    args = ['rumur', '--output', '/dev/stdout', self.model]              \
      + (['--debug'] if self.debug else [])                              \
      + (['--output-format', 'machine-readable'] if self.xml else [])    \
      + (['--threads', '2'] if self.multithreaded and CPUS == 1 else []) \
      + (['--threads', '1'] if not self.multithreaded else [])           \
      + self.rumur_flags

    if config['HAS_VALGRIND']:
      args = ['valgrind', '--leak-check=full', '--show-leak-kinds=all',
        '--error-exitcode=42'] + args

    # call rumur
    ret, stdout, stderr = run(args)
    if config['HAS_VALGRIND']:
      if ret == 42:
        return Fail(f'Memory leak:\n{stdout}{stderr}')
    if ret != self.rumur_exit_code:
      return Fail(f'Rumur failed:\n{stdout}{stderr}')

    # if we expected to fail, we are done
    if ret != 0: return

    model_c = stdout

    with TemporaryDirectory() as tmp:

      # build up arguments to call the C compiler
      model_bin = os.path.join(tmp, 'model.exe')
      args = [config['CC']] + config['C_FLAGS'] + ['-o', model_bin, '-',
        '-lpthread']

      if config['NEEDS_LIBATOMIC']:
        args.append('-latomic')

      # call the C compiler
      ret, stdout, stderr = run(args, model_c)
      if ret != 0:
        return Fail(f'C compilation failed:\n{stdout}{stderr}')

      # now run the model itself
      ret, stdout, stderr = run([model_bin])
      if ret != self.checker_exit_code:
        return Fail(f'Unexpected checker exit status {ret}:\n{stdout}{stderr}')

    # if the test has a stdout expectation, check that now
    if self.checker_output is not None:
      if self.checker_output.search(stdout) is None:
        return Fail( 'Checker output did not match expectation regex:\n'
                    f'{stdout}{stderr}')

    # coarse grained check for whether the model contains a 'put' statement that
    # could screw up XML validation
    with open(self.model, 'rt', encoding='utf-8') as f:
      has_put = re.search(r'\bput\b', f.read()) is not None

    if self.xml and not has_put:

      model_xml = stdout

      if not config['HAS_XMLLINT']: return Skip('xmllint not available')

      # validate the XML
      args = ['xmllint', '--relaxng', VERIFIER_RNG, '--noout', '-']
      ret, stdout, stderr = run(args, model_xml)
      if ret != 0:
        return Fail( 'Failed to XML-validate machine reachable output:\n'
                    f'{stdout}{stderr}')

class Executable(Test):
  def __init__(self, exe: str):
    self.exe = exe
  def description(self) -> str: return f'----- exec {os.path.basename(self.exe)}'
  def run(self) -> Result:
    ret, stdout, stderr = run(self.exe)
    output = f'{stdout}{stderr}'
    return None                 if ret == 0 else \
           Skip(output.strip()) if ret == 125 else \
           Fail(output)

class Murphi2CTest(Tweakable):
  def __init__(self, model: str):
    super().__init__()
    self.model = model
    self.xml = False # dummy setting that tests might reference
  def description(self) -> str:
    return f'----{"V" if config["HAS_VALGRIND"] else " "} ' \
           f'murphi2c {os.path.basename(self.model)}'
  def run(self) -> Result:

    self.apply_options(self.model)

    # there is no C equivalent of isundefined, because an implicit assumption in
    # the C representation is that you do not rely on undefined values
    with open(self.model, 'rt', encoding='utf-8') as f:
      should_fail = re.search(r'\bisundefined\b', f.read()) is not None

    args = ['murphi2c', self.model]
    if config['HAS_VALGRIND']:
      args = ['valgrind', '--leak-check=full', '--show-leak-kinds=all',
        '--error-exitcode=42'] + args
    ret, stdout, stderr = run(args)
    if config['HAS_VALGRIND']:
      if ret == 42:
        return Fail(f'Memory leak:\n{stdout}{stderr}')

    # if rumur was expected to reject this model, we allow murphi2c to fail
    if self.rumur_exit_code == 0 and not should_fail and ret != 0:
      return Fail(f'Unexpected murphi2c exit status {ret}:\n{stdout}{stderr}')

    if should_fail and ret == 0:
      return Fail(f'Unexpected murphi2c exit status {ret}:\n{stdout}{stderr}')

    if ret != 0:
      return None

    # ask the C compiler if this is valid
    args = [config['CC']] + config['C_FLAGS'] + ['-c', '-o', os.devnull, '-']
    ret, stdout, stderr = run(args, stdout)
    if ret != 0:
      return Fail(f'C compilation failed:\n{stdout}{stderr}')

class Murphi2CHeaderTest(Tweakable):
  def __init__(self, model: str):
    super().__init__()
    self.model = model
    self.xml = False # dummy setting that tests might reference
  def description(self) -> str:
    return f'----{"V" if config["HAS_VALGRIND"] else " "} ' \
           f'murphi2c --header {os.path.basename(self.model)}'
  def run(self) -> Result:

    self.apply_options(self.model)

    # there is no C equivalent of isundefined, because an implicit assumption in
    # the C representation is that you do not rely on undefined values
    with open(self.model, 'rt', encoding='utf-8') as f:
      should_fail = re.search(r'\bisundefined\b', f.read()) is not None

    args = ['murphi2c', '--header', self.model]
    if config['HAS_VALGRIND']:
      args = ['valgrind', '--leak-check=full', '--show-leak-kinds=all',
        '--error-exitcode=42'] + args
    ret, stdout, stderr = run(args)
    if config['HAS_VALGRIND']:
      if ret == 42:
        return Fail(f'Memory leak:\n{stdout}{stderr}')

    # if rumur was expected to reject this model, we allow murphi2c to fail
    if self.rumur_exit_code == 0 and not should_fail and ret != 0:
      return Fail(f'Unexpected murphi2c exit status {ret}:\n{stdout}{stderr}')

    if should_fail and ret == 0:
      return Fail(f'Unexpected murphi2c exit status {ret}:\n{stdout}{stderr}')

    if ret != 0:
      return None

    with TemporaryDirectory() as tmp:

      # write the header to a temporary file
      header = os.path.join(tmp, 'header.h')
      with open(header, 'wt', encoding='utf-8') as f:
        f.write(stdout)

      # ask the C compiler if the header is valid
      main_c = f'#include "{header}"\nint main(void) {{ return 0; }}\n'
      args = [config['CC']] + config['C_FLAGS'] + ['-o', os.devnull, '-']
      ret, stdout, stderr = run(args, main_c)
      if ret != 0:
        return Fail(f'C compilation failed:\n{stdout}{stderr}')

      # ask the C++ compiler if it is valid there too
      ret, stdout, stderr = run([config['CXX'], '-std=c++11', '-o', os.devnull,
        '-x', 'c++', '-', '-Werror=format', '-Werror=sign-compare',
        '-Werror=type-limits'], main_c)
      if ret != 0:
        return Fail(f'C++ compilation failed:\n{stdout}{stderr}')

class Murphi2XMLTest(Tweakable):
  def __init__(self, model: str):
    super().__init__()
    self.model = model
    self.xml = False # dummy setting that tests might reference
  def description(self) -> str:
    return f'----{"V" if config["HAS_VALGRIND"] else " "} ' \
           f'murphi2xml {os.path.basename(self.model)}'
  def run(self) -> Result:

    self.apply_options(self.model)

    args = ['murphi2xml', self.model]
    if config['HAS_VALGRIND']:
      args = ['valgrind', '--leak-check=full', '--show-leak-kinds=all',
        '--error-exitcode=42'] + args
    ret, stdout, stderr = run(args)
    if config['HAS_VALGRIND']:
      if ret == 42:
        return Fail(f'Memory leak:\n{stdout}{stderr}')

    # if rumur was expected to reject this model, we allow murphi2xml to fail
    if self.rumur_exit_code == 0 and ret != 0:
      return Fail(f'Unexpected murphi2xml exit status {ret}:\n{stdout}{stderr}')

    if ret != 0:
      return None

    # murphi2xml will have written XML to its stdout
    xmlcontent = stdout

    # See if we have xmllint
    if not config['HAS_XMLLINT']:
      return Skip('xmllint not available for validation')

    # Validate the XML
    ret, stdout, stderr = run(['xmllint', '--relaxng', MURPHI2XML_RNG,
      '--noout', '-'], xmlcontent)
    if ret != 0:
      return Fail(f'Failed to validate:\n{stdout}{stderr}')

def check(test: Test) -> int:
  'run a test case and report its result'

  result = test.run()

  if result is None:
    pr(f'{GREEN}PASS{RESET} {test.description()}\n')
    return 1, 0, 0
  elif isinstance(result, Skip):
    pr(f'{YELLOW}SKIP{RESET} {test.description()} [{result.reason}]\n')
    return 0, 1, 0
  else:
    assert isinstance(result, Fail)
    pr(f'{RED}FAIL{RESET} {test.description()}\n{result.output}')
    return 0, 0, 1

def main(args: [str]) -> int:

  # setup stdout to make encoding errors non-fatal
  sys.stdout = codecs.getwriter('utf-8')(sys.stdout.buffer, 'replace')

  parser = argparse.ArgumentParser(description='Rumur test suite')
  parser.add_argument('--jobs', '-j', type=int, help='number of threads to use',
    default=CPUS)
  parser.add_argument('testcase', nargs='*',
    help='specific test case(s) to run')
  options = parser.parse_args(args[1:])

  # parse configuration
  global config
  manager = multiprocessing.Manager()
  config = manager.dict()
  for p in sorted(os.listdir(os.path.join(os.path.dirname(__file__), 'config'))):
    path = os.path.join(os.path.dirname(__file__), 'config', p)

    # skip subdirectories
    if os.path.isdir(path): continue

    # skip non-executable files
    if not os.access(path, os.X_OK): continue

    config[p] = eval(sp.check_output([path]))

  pool = multiprocessing.Pool(options.jobs)

  index = 1
  tests = []

  # find files in our directory
  root = os.path.dirname(__file__)
  for stem in sorted(os.listdir(root)):
    path = os.path.join(root, stem)

    # skip if this does not match the user’s filter
    if len(options.testcase) > 0 and stem not in options.testcase: continue

    # skip directories
    if os.path.isdir(path): continue

    # skip ourselves
    if path == __file__: continue

    # if this is executable, treat it as a test case
    if os.access(path, os.X_OK):
      if in_range(index):
        tests.append(Executable(path))
      index += 1

    # if this is not a model, skip the remaining generic logic
    if os.path.splitext(path)[-1] != '.m': continue

    for debug, optimised, multithreaded, xml \
        in itertools.product((False, True), repeat=4):

      # debug output causes invalid XML, so skip
      if debug and xml: continue

      if in_range(index):
        tests.append(Model(path, debug, optimised, multithreaded, xml))
      index += 1

    if in_range(index):
      tests.append(Murphi2XMLTest(path))
    index += 1

    if in_range(index):
      tests.append(Murphi2CTest(path))
    index += 1

    if in_range(index):
      tests.append(Murphi2CHeaderTest(path))
    index += 1

  if len(tests) == 0:
    pr(f'no tests found\n')
    return -1

  pr( 'Configuration:\n'
     f'  CPUS = {CPUS}\n'
     f'  STDOUT_ISATTY = {STDOUT_ISATTY}\n'
     f'  MIN_TEST = {MIN_TEST}\n'
     f'  MAX_TEST = {MAX_TEST}\n')
  for key, value in config.items():
    pr(f'  {key} = {value}\n')
  pr('\n')

  pr(f'Running {len(tests)} tests using {options.jobs} threads...\n'
      '     +------ debug\n'
      '     |+----- optimised\n'
      '     ||+---- multithreaded\n'
      '     |||+--- XML\n'
      '     ||||+-- Valgrind\n')

  # run all tests in parallel and accumulate the results
  passed, skipped, failed = map(sum, zip(*pool.imap_unordered(check, tests)))

  pr(f'{passed} passed, {skipped} skipped, {failed} failed '
     f'out of {len(tests)} total tests\n')

  return 0 if failed == 0 else 1

if __name__ == '__main__':
  sys.exit(main(sys.argv))
