#!/usr/bin/env python3

'''
Generate contents of a version.cc.
'''

import os
import re
import subprocess as sp
import sys
from typing import Optional

# The version of the last tagged release of Rumur. This will be used as the
# version number if no Git information is available.
LAST_RELEASE = 'v2020.05.18'

def has_git() -> bool:
  '''
  Return True if we are in a Git repository and have Git.
  '''

  # Return False if we don't have Git.
  try:
    sp.check_call(['which', 'git'], stdout=sp.DEVNULL, stderr=sp.DEVNULL)
  except:
    return False

  # Return False if we have no Git repository information.
  if not os.path.exists(os.path.join(os.path.dirname(__file__),
      '..', '..', '.git')):
    return False

  return True

def get_tag() -> Optional[str]:
  '''
  Find the version tag of the current Git commit, e.g. v2020.05.03, if it
  exists.
  '''
  try:
    tag = sp.check_output(['git', 'describe', '--tags'], stderr=sp.DEVNULL)
  except sp.CalledProcessError:
    tag = None

  if tag is not None:
    tag = tag.decode('utf-8', 'replace').strip()
    if re.match(r'v[\d\.]+$', tag) is None:
      # Not a version tag.
      tag = None

  return tag

def get_sha() -> str:
  '''
  Find the hash of the current Git commit.
  '''
  rev = sp.check_output(['git', 'rev-parse', '--verify', 'HEAD'])
  rev = rev.decode('utf-8', 'replace').strip()

  return rev

def is_dirty() -> bool:
  '''
  Determine whether the current working directory has uncommitted changes.
  '''
  dirty = False

  p = sp.run(['git', 'diff', '--exit-code'], stdout=sp.DEVNULL,
    stderr=sp.DEVNULL)
  dirty |= p.returncode != 0

  p = sp.Popen(['git', 'diff', '--cached', '--exit-code'], stdout=sp.DEVNULL,
    stderr=sp.DEVNULL)
  dirty |= p.returncode != 0

  return dirty

def main(args: [str]) -> int:

  if len(args) != 2 or args[1] == '--help':
    sys.stderr.write(
      f'usage: {args[0]} file\n'
       ' write version information as a C++ source file\n')
    return -1

  # Get the contents of the old version file if it exists.
  old = None
  if os.path.exists(args[1]):
    with open(args[1], 'rt') as f:
      old = f.read()

  version = None

  # first, look for an environment variable that overrides other version sources
  version = os.environ.get('RUMUR_VERSION')

  # second, look for a version tag on the current commit
  if version is None and has_git():
    tag = get_tag()
    if tag is not None:
      version = f'{tag}{" (dirty)" if is_dirty() else ""}'

  # third, look for the commit hash as the version
  if version is None and has_git():
    rev = get_sha()
    assert rev is not None
    version = f'Git commit {rev}{" (dirty)" if is_dirty() else ""}'

  # Finally, fall back to our known release version.
  if version is None:
    version = LAST_RELEASE

  new =  '#pragma once\n' \
         '\n' \
         'namespace rumur {\n' \
         '\n' \
         'static constexpr const char *get_version() {\n' \
        f'  return "{version}";\n' \
         '}\n' \
         '\n' \
         '}'

  # If the version has changed, update the output. Otherwise we leave the old
  # contents -- and more importantly, the timestamp -- intact.
  if old != new:
    with open(args[1], 'wt') as f:
      f.write(new)

  return 0

if __name__ == '__main__':
  sys.exit(main(sys.argv))
