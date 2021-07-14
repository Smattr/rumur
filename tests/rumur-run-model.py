#!/usr/bin/env python3

'test that rumur-run can check a basic model'

import os
import platform
import subprocess as sp
import sys

if platform.system() == 'Linux' and 'TRAVIS' in os.environ:
  release = sp.check_output(['lsb_release', '--release', '--short'],
    universal_newlines=True).strip()
  if release == '14.04':
    if os.environ.get('CC') in ('gcc-7', 'gcc-8', 'gcc-9'):
      # XXX: When using GCC ≥7 on Ubuntu 14.04 (as we do in CI) and the compiler
      # is passed -march=native -mtune=native, it generates assembly sequences
      # that GAS from Binutils 2.24 (the default on this platform) is unable to
      # comprehend. It is possible to install Binutils 2.26 but in Travis it
      # seems flaky as to whether this works. To avoid false failures, just skip
      # this test in that configuration.
      print('this test is flaky on Ubuntu 14.04 with GCC 7+')
      sys.exit(125)


RUMUR_RUN = os.path.join(os.path.dirname(__file__), '../rumur/src/rumur-run')

MODEL = '''
var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
'''

def main():
  p = sp.Popen(['python3', RUMUR_RUN], stdin=sp.PIPE)
  p.communicate(MODEL.encode('utf-8', 'replace'))
  return p.returncode

if __name__ == '__main__':
  sys.exit(main())
